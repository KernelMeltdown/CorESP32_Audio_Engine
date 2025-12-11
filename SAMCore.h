// SAMCore.h - SAM Core Structures and Definitions
// ESP32 Bare-Metal Speech Synthesis

#ifndef SAMCORE_H
#define SAMCORE_H

#include <Arduino.h>
#include "SAMConfig.h"

// ============================================================================
// PHONEME DATA STRUCTURE
// ============================================================================

struct SAMPhonemeData {
    uint8_t index;           // Phoneme index (0-63)
    uint8_t type;            // PHONEME_VOWEL, PHONEME_CONSONANT, etc.
    uint8_t duration;        // Duration in base timing units
    uint8_t stress;          // Stress level (0-2)
    
    // Formant parameters
    float f1, f2, f3;        // Formant frequencies (Hz)
    float a1, a2, a3;        // Formant amplitudes (0.0-1.0)
    float bw;                // Bandwidth (Hz)
};

// ============================================================================
// VOICE PARAMETERS
// ============================================================================

struct SAMVoiceParams {
    uint8_t speed;           // 40-150
    uint8_t pitch;           // 20-120
    uint8_t throat;          // 90-180
    uint8_t mouth;           // 90-180
    
    // DSP enhancements
    uint8_t smoothing;       // 0-100
    uint8_t interpolation;   // 0-100
    uint8_t formantBoost;    // 0-50
    int8_t  bassControl;     // -10 to +10 dB
    
    SAMVoiceParams() : 
        speed(SAM_DEFAULT_SPEED),
        pitch(SAM_DEFAULT_PITCH),
        throat(SAM_DEFAULT_THROAT),
        mouth(SAM_DEFAULT_MOUTH),
        smoothing(SAM_SMOOTH_AMOUNT),
        interpolation(SAM_INTERP_AMOUNT),
        formantBoost(SAM_FORMANT_BOOST),
        bassControl(SAM_BASS_DB)
    {}
};

// ============================================================================
// SYNTHESIS STATE
// ============================================================================

struct SAMSynthState {
    SAMPhonemeData phonemes[SAM_PHONEME_BUFFER];
    size_t phonemeCount;
    size_t currentPhoneme;
    size_t sampleOffset;
    bool active;
    
    SAMSynthState() : 
        phonemeCount(0),
        currentPhoneme(0),
        sampleOffset(0),
        active(false)
    {}
};

// ============================================================================
// FORMANT STATE (for continuous synthesis)
// ============================================================================

struct SAMFormantState {
    float phase1, phase2, phase3;  // Oscillator phases
    float freq1, freq2, freq3;     // Current frequencies
    float amp1, amp2, amp3;        // Current amplitudes
    float targetFreq1, targetFreq2, targetFreq3;
    float targetAmp1, targetAmp2, targetAmp3;
    
    SAMFormantState() {
        reset();
    }
    
    void reset() {
        phase1 = phase2 = phase3 = 0.0f;
        freq1 = freq2 = freq3 = 0.0f;
        amp1 = amp2 = amp3 = 0.0f;
        targetFreq1 = targetFreq2 = targetFreq3 = 0.0f;
        targetAmp1 = targetAmp2 = targetAmp3 = 0.0f;
    }
};

// ============================================================================
// SAM CORE CLASS
// ============================================================================

class SAMCore {
private:
    SAMVoiceParams params;
    SAMSynthState state;
    SAMFormantState formantState;
    
public:
    SAMCore();
    
    // Voice parameter control
    void setSpeed(uint8_t speed);
    void setPitch(uint8_t pitch);
    void setThroat(uint8_t throat);
    void setMouth(uint8_t mouth);
    void setParams(const SAMVoiceParams& p);
    SAMVoiceParams& getParams() { return params; }
    
    // Synthesis control
    bool synthesize(const char* text);
    size_t render(int16_t* buffer, size_t samples);
    void reset();
    bool isActive() const { return state.active; }
    
    // Status
    size_t getPhonemeCount() const { return state.phonemeCount; }
    size_t getCurrentPhoneme() const { return state.currentPhoneme; }
    float getProgress() const {
        if (state.phonemeCount == 0) return 0.0f;
        return (float)state.currentPhoneme / (float)state.phonemeCount;
    }
};

#endif // SAMCORE_H
