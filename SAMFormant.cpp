// SAMFormant.cpp - Formant Synthesis Implementation

#include "SAMFormant.h"
#include <math.h>

// ============================================================================
// VOICE PARAMETER MODIFIERS
// ============================================================================

float SAMFormant::pitchModifier(uint8_t pitch) {
    // Map pitch (20-120) to frequency multiplier (0.5-2.0)
    // 64 = 1.0 (neutral)
    float normalized = (pitch - 64.0f) / 64.0f;
    return powf(2.0f, normalized);  // Exponential scaling
}

float SAMFormant::throatModifier(uint8_t throat) {
    // Throat affects F1 (lower formant)
    // 128 = 1.0 (neutral)
    return 0.5f + (throat / 256.0f);
}

float SAMFormant::mouthModifier(uint8_t mouth) {
    // Mouth affects F2/F3 (higher formants)
    // 128 = 1.0 (neutral)
    return 0.7f + (mouth / 256.0f);
}

// ============================================================================
// APPLY VOICE PARAMETERS
// ============================================================================

void SAMFormant::applyVoiceParams(
    struct SAMPhonemeData* phonemes,
    size_t count,
    const SAMVoiceParams& params
) {
    float pitchMod = pitchModifier(params.pitch);
    float throatMod = throatModifier(params.throat);
    float mouthMod = mouthModifier(params.mouth);
    
    for (size_t i = 0; i < count; i++) {
        // Adjust formant frequencies
        phonemes[i].f1 *= throatMod * pitchMod;
        phonemes[i].f2 *= mouthMod * pitchMod;
        phonemes[i].f3 *= mouthMod * pitchMod;
        
        // Clamp to valid ranges
        phonemes[i].f1 = constrain(phonemes[i].f1, FORMANT_F1_MIN, FORMANT_F1_MAX);
        phonemes[i].f2 = constrain(phonemes[i].f2, FORMANT_F2_MIN, FORMANT_F2_MAX);
        phonemes[i].f3 = constrain(phonemes[i].f3, FORMANT_F3_MIN, FORMANT_F3_MAX);
        
        // Adjust duration based on speed
        // Speed: 40 = slow (2x), 72 = normal, 150 = fast (0.5x)
        float speedMod = 72.0f / params.speed;
        phonemes[i].duration = (uint8_t)(phonemes[i].duration * speedMod);
        
        if (phonemes[i].duration < 1) {
            phonemes[i].duration = 1;
        }
    }
}

// ============================================================================
// FORMANT OSCILLATOR
// ============================================================================

float SAMFormant::generateFormant(
    float frequency,
    float amplitude,
    float& phase,
    float sampleRate
) {
    if (amplitude < 0.001f || frequency < 1.0f) {
        return 0.0f;
    }
    
    // Generate sine wave using ESP32 hardware FPU
    float output = sinf(phase * 2.0f * PI) * amplitude;
    
    // Increment phase
    phase += frequency / sampleRate;
    
    // Wrap phase
    if (phase >= 1.0f) {
        phase -= floorf(phase);
    }
    
    return output;
}

// ============================================================================
// PHONEME INTERPOLATION
// ============================================================================

void SAMFormant::interpolateFormants(
    const struct SAMPhonemeData& from,
    const struct SAMPhonemeData& to,
    float t,
    float& f1, float& f2, float& f3,
    float& a1, float& a2, float& a3
) {
    // Linear interpolation
    f1 = from.f1 + (to.f1 - from.f1) * t;
    f2 = from.f2 + (to.f2 - from.f2) * t;
    f3 = from.f3 + (to.f3 - from.f3) * t;
    
    a1 = from.a1 + (to.a1 - from.a1) * t;
    a2 = from.a2 + (to.a2 - from.a2) * t;
    a3 = from.a3 + (to.a3 - from.a3) * t;
}
