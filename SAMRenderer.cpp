// SAMRenderer.cpp - Audio Sample Renderer Implementation

#include "SAMRenderer.h"
#include "SAMFormant.h"
#include <math.h>

// ============================================================================
// MAIN RENDER FUNCTION
// ============================================================================

size_t SAMRenderer::render(
    const struct SAMPhonemeData* phonemes,
    size_t phonemeCount,
    size_t& currentPhoneme,
    size_t& sampleOffset,
    struct SAMFormantState& formantState,
    const SAMVoiceParams& params,
    int16_t* buffer,
    size_t samples
) {
    if (!phonemes || !buffer || samples == 0 || currentPhoneme >= phonemeCount) {
        return 0;
    }
    
    size_t samplesRendered = 0;
    
    while (samplesRendered < samples && currentPhoneme < phonemeCount) {
        const struct SAMPhonemeData& phoneme = phonemes[currentPhoneme];
        
        // Calculate phoneme duration in samples
        uint32_t phonemeSamples = (uint32_t)phoneme.duration * SAM_TIMING_BASE;
        
        // Check if we need transition to next phoneme
        bool inTransition = (sampleOffset + SAM_TRANSITION_SAMPLES >= phonemeSamples) &&
                           (currentPhoneme + 1 < phonemeCount);
        
        size_t toRender = samples - samplesRendered;
        size_t rendered;
        
        if (inTransition) {
            // Render transition
            rendered = renderTransition(
                phoneme,
                phonemes[currentPhoneme + 1],
                sampleOffset,
                formantState,
                params,
                buffer + samplesRendered,
                toRender
            );
        } else {
            // Render normal phoneme
            rendered = renderPhoneme(
                phoneme,
                sampleOffset,
                formantState,
                params,
                buffer + samplesRendered,
                toRender
            );
        }
        
        samplesRendered += rendered;
        sampleOffset += rendered;
        
        // Move to next phoneme?
        if (sampleOffset >= phonemeSamples) {
            currentPhoneme++;
            sampleOffset = 0;
            
            // Update formant state for next phoneme
            if (currentPhoneme < phonemeCount) {
                const struct SAMPhonemeData& next = phonemes[currentPhoneme];
                formantState.targetFreq1 = next.f1;
                formantState.targetFreq2 = next.f2;
                formantState.targetFreq3 = next.f3;
                formantState.targetAmp1 = next.a1;
                formantState.targetAmp2 = next.a2;
                formantState.targetAmp3 = next.a3;
            }
        }
    }
    
    // Apply DSP enhancements
    #if SAM_ENABLE_SMOOTHING
    if (params.smoothing > 0) {
        applySmoothingFilter(buffer, samplesRendered, params.smoothing);
    }
    #endif
    
    #if SAM_ENABLE_INTERPOLATION
    if (params.interpolation > 0) {
        applyInterpolation(buffer, samplesRendered, params.interpolation);
    }
    #endif
    
    #if SAM_ENABLE_FORMANT_BOOST
    if (params.formantBoost > 0) {
        applyFormantBoost(buffer, samplesRendered, params.formantBoost);
    }
    #endif
    
    #if SAM_ENABLE_BASS_CONTROL
    if (params.bassControl != 0) {
        applyBassControl(buffer, samplesRendered, params.bassControl);
    }
    #endif
    
    // Remove DC offset
    removeDCOffset(buffer, samplesRendered);
    
    return samplesRendered;
}

// ============================================================================
// PHONEME RENDERING
// ============================================================================

size_t SAMRenderer::renderPhoneme(
    const struct SAMPhonemeData& phoneme,
    size_t& sampleOffset,
    struct SAMFormantState& formantState,
    const SAMVoiceParams& params,
    int16_t* buffer,
    size_t samples
) {
    uint32_t phonemeSamples = (uint32_t)phoneme.duration * SAM_TIMING_BASE;
    size_t toRender = min(samples, (size_t)(phonemeSamples - sampleOffset));
    
    // Set target formants if first sample of phoneme
    if (sampleOffset == 0) {
        formantState.targetFreq1 = phoneme.f1;
        formantState.targetFreq2 = phoneme.f2;
        formantState.targetFreq3 = phoneme.f3;
        formantState.targetAmp1 = phoneme.a1;
        formantState.targetAmp2 = phoneme.a2;
        formantState.targetAmp3 = phoneme.a3;
    }
    
    // Render samples
    for (size_t i = 0; i < toRender; i++) {
        // Smooth formant parameter changes
        formantState.freq1 += (formantState.targetFreq1 - formantState.freq1) * 0.1f;
        formantState.freq2 += (formantState.targetFreq2 - formantState.freq2) * 0.1f;
        formantState.freq3 += (formantState.targetFreq3 - formantState.freq3) * 0.1f;
        formantState.amp1 += (formantState.targetAmp1 - formantState.amp1) * 0.1f;
        formantState.amp2 += (formantState.targetAmp2 - formantState.amp2) * 0.1f;
        formantState.amp3 += (formantState.targetAmp3 - formantState.amp3) * 0.1f;
        
        // Generate formants
        float sample = 0.0f;
        
        sample += SAMFormant::generateFormant(
            formantState.freq1,
            formantState.amp1,
            formantState.phase1,
            SAM_SAMPLE_RATE
        );
        
        sample += SAMFormant::generateFormant(
            formantState.freq2,
            formantState.amp2,
            formantState.phase2,
            SAM_SAMPLE_RATE
        );
        
        sample += SAMFormant::generateFormant(
            formantState.freq3,
            formantState.amp3,
            formantState.phase3,
            SAM_SAMPLE_RATE
        );
        
        // Convert to 16-bit
        buffer[i] = (int16_t)(sample * 10000.0f);
    }
    
    return toRender;
}

// ============================================================================
// TRANSITION RENDERING
// ============================================================================

size_t SAMRenderer::renderTransition(
    const struct SAMPhonemeData& from,
    const struct SAMPhonemeData& to,
    size_t& sampleOffset,
    struct SAMFormantState& formantState,
    const SAMVoiceParams& params,
    int16_t* buffer,
    size_t samples
) {
    uint32_t phonemeSamples = (uint32_t)from.duration * SAM_TIMING_BASE;
    size_t transitionStart = phonemeSamples - SAM_TRANSITION_SAMPLES;
    size_t toRender = min(samples, (size_t)(phonemeSamples - sampleOffset));
    
    for (size_t i = 0; i < toRender; i++) {
        // Calculate transition progress (0.0 to 1.0)
        float t = (float)(sampleOffset + i - transitionStart) / (float)SAM_TRANSITION_SAMPLES;
        t = constrain(t, 0.0f, 1.0f);
        
        // Interpolate formants
        float f1, f2, f3, a1, a2, a3;
        SAMFormant::interpolateFormants(from, to, t, f1, f2, f3, a1, a2, a3);
        
        // Update targets with interpolated values
        formantState.targetFreq1 = f1;
        formantState.targetFreq2 = f2;
        formantState.targetFreq3 = f3;
        formantState.targetAmp1 = a1;
        formantState.targetAmp2 = a2;
        formantState.targetAmp3 = a3;
        
        // Smooth transitions
        formantState.freq1 += (formantState.targetFreq1 - formantState.freq1) * 0.2f;
        formantState.freq2 += (formantState.targetFreq2 - formantState.freq2) * 0.2f;
        formantState.freq3 += (formantState.targetFreq3 - formantState.freq3) * 0.2f;
        formantState.amp1 += (formantState.targetAmp1 - formantState.amp1) * 0.2f;
        formantState.amp2 += (formantState.targetAmp2 - formantState.amp2) * 0.2f;
        formantState.amp3 += (formantState.targetAmp3 - formantState.amp3) * 0.2f;
        
        // Generate formants
        float sample = 0.0f;
        
        sample += SAMFormant::generateFormant(
            formantState.freq1,
            formantState.amp1,
            formantState.phase1,
            SAM_SAMPLE_RATE
        );
        
        sample += SAMFormant::generateFormant(
            formantState.freq2,
            formantState.amp2,
            formantState.phase2,
            SAM_SAMPLE_RATE
        );
        
        sample += SAMFormant::generateFormant(
            formantState.freq3,
            formantState.amp3,
            formantState.phase3,
            SAM_SAMPLE_RATE
        );
        
        // Convert to 16-bit
        buffer[i] = (int16_t)(sample * 10000.0f);
    }
    
    return toRender;
}

// ============================================================================
// DSP FILTERS
// ============================================================================

void SAMRenderer::applySmoothingFilter(int16_t* buffer, size_t samples, uint8_t amount) {
    if (samples < 3 || amount == 0) return;
    
    float factor = amount / 100.0f;
    
    // Simple 3-point moving average
    for (size_t i = 1; i < samples - 1; i++) {
        int32_t smoothed = (buffer[i-1] + buffer[i] * 2 + buffer[i+1]) / 4;
        buffer[i] = (int16_t)((1.0f - factor) * buffer[i] + factor * smoothed);
    }
}

void SAMRenderer::applyInterpolation(int16_t* buffer, size_t samples, uint8_t amount) {
    if (samples < 4 || amount == 0) return;
    
    float factor = amount / 100.0f;
    
    // Cubic interpolation for smooth transitions
    for (size_t i = 2; i < samples - 2; i++) {
        // Catmull-Rom cubic interpolation
        float p0 = buffer[i-2];
        float p1 = buffer[i-1];
        float p2 = buffer[i];
        float p3 = buffer[i+1];
        
        float interpolated = p1 + 0.5f * (p2 - p0 + 
                            (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3 +
                            (3.0f * (p1 - p2) + p3 - p0) * 0.5f) * 0.5f);
        
        buffer[i] = (int16_t)((1.0f - factor) * buffer[i] + factor * interpolated);
    }
}

void SAMRenderer::applyFormantBoost(int16_t* buffer, size_t samples, uint8_t amount) {
    if (samples == 0 || amount == 0) return;
    
    float boost = 1.0f + (amount / 100.0f) * 0.3f;
    
    for (size_t i = 0; i < samples; i++) {
        int32_t boosted = (int32_t)(buffer[i] * boost);
        buffer[i] = (int16_t)constrain(boosted, -32768, 32767);
    }
}

void SAMRenderer::applyBassControl(int16_t* buffer, size_t samples, int8_t dB) {
    if (samples == 0 || dB == 0) return;
    
    // Simple bass boost/cut using first-order IIR filter
    float gain = powf(10.0f, dB / 20.0f);
    static float prev = 0.0f;
    
    for (size_t i = 0; i < samples; i++) {
        float input = buffer[i];
        float lowFreq = (input + prev) * 0.5f;
        prev = input;
        
        float output = input + (lowFreq * (gain - 1.0f));
        buffer[i] = (int16_t)constrain((int32_t)output, -32768, 32767);
    }
}

void SAMRenderer::removeDCOffset(int16_t* buffer, size_t samples) {
    if (samples == 0) return;
    
    // Calculate average
    int32_t sum = 0;
    for (size_t i = 0; i < samples; i++) {
        sum += buffer[i];
    }
    
    int16_t offset = (int16_t)(sum / (int32_t)samples);
    
    // Remove offset
    if (offset != 0) {
        for (size_t i = 0; i < samples; i++) {
            buffer[i] -= offset;
        }
    }
}
