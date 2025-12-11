// SAMFormant.h - Formant Synthesis Engine

#ifndef SAMFORMANT_H
#define SAMFORMANT_H

#include <Arduino.h>
#include "SAMCore.h"

// ============================================================================
// FORMANT SYNTHESIS ENGINE
// ============================================================================

class SAMFormant {
public:
    // Apply voice parameters to phoneme formants
    static void applyVoiceParams(
        struct SAMPhonemeData* phonemes,
        size_t count,
        const SAMVoiceParams& params
    );
    
    // Generate single formant oscillator sample
    static float generateFormant(
        float frequency,
        float amplitude,
        float& phase,
        float sampleRate = SAM_SAMPLE_RATE
    );
    
    // Smooth transition between phonemes
    static void interpolateFormants(
        const struct SAMPhonemeData& from,
        const struct SAMPhonemeData& to,
        float t,
        float& f1, float& f2, float& f3,
        float& a1, float& a2, float& a3
    );
    
private:
    // Modifiers based on voice parameters
    static float pitchModifier(uint8_t pitch);
    static float throatModifier(uint8_t throat);
    static float mouthModifier(uint8_t mouth);
};

#endif // SAMFORMANT_H
