// SAMRenderer.h - Audio Sample Renderer

#ifndef SAMRENDERER_H
#define SAMRENDERER_H

#include <Arduino.h>
#include "SAMCore.h"

// ============================================================================
// AUDIO RENDERER
// ============================================================================

class SAMRenderer {
public:
    // Main rendering function
    static size_t render(
        const struct SAMPhonemeData* phonemes,
        size_t phonemeCount,
        size_t& currentPhoneme,
        size_t& sampleOffset,
        struct SAMFormantState& formantState,
        const SAMVoiceParams& params,
        int16_t* buffer,
        size_t samples
    );
    
private:
    // Generate samples for current phoneme
    static size_t renderPhoneme(
        const struct SAMPhonemeData& phoneme,
        size_t& sampleOffset,
        struct SAMFormantState& formantState,
        const SAMVoiceParams& params,
        int16_t* buffer,
        size_t samples
    );
    
    // Transition between phonemes
    static size_t renderTransition(
        const struct SAMPhonemeData& from,
        const struct SAMPhonemeData& to,
        size_t& sampleOffset,
        struct SAMFormantState& formantState,
        const SAMVoiceParams& params,
        int16_t* buffer,
        size_t samples
    );
    
    // DSP processing
    static void applySmoothingFilter(
        int16_t* buffer,
        size_t samples,
        uint8_t amount
    );
    
    static void applyInterpolation(
        int16_t* buffer,
        size_t samples,
        uint8_t amount
    );
    
    static void applyFormantBoost(
        int16_t* buffer,
        size_t samples,
        uint8_t amount
    );
    
    static void applyBassControl(
        int16_t* buffer,
        size_t samples,
        int8_t dB
    );
    
    // DC offset removal
    static void removeDCOffset(
        int16_t* buffer,
        size_t samples
    );
};

#endif // SAMRENDERER_H
