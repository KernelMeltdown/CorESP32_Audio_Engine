/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM DSP PROCESSOR - Header                                                  ║
 ║  Digital Signal Processing for SAM Speech Synthesis                         ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#ifndef SAM_DSP_PROCESSOR_H
#define SAM_DSP_PROCESSOR_H

#include <Arduino.h>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════
// Biquad Filter Coefficients
// ═══════════════════════════════════════════════════════════════════════════

struct BiquadCoeffs {
    float b0, b1, b2;  // Feedforward coefficients
    float a1, a2;      // Feedback coefficients
    
    BiquadCoeffs() : b0(1), b1(0), b2(0), a1(0), a2(0) {}
};

struct BiquadState {
    float x1, x2;  // Input history
    float y1, y2;  // Output history
    
    BiquadState() : x1(0), x2(0), y1(0), y2(0) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// SAM DSP Processor Class
// ═══════════════════════════════════════════════════════════════════════════

class SAMDSPProcessor {
public:
    SAMDSPProcessor();
    ~SAMDSPProcessor();
    
    // Smoothing
    void applySmoothing(float* buffer, size_t samples, float amount = 0.3f);
    
    // Interpolation
    void applyCubicInterpolation(float* buffer, size_t samples, float amount = 0.5f);
    
    // Formant enhancement
    void applyFormantBoost(float* buffer, size_t samples, 
                          float freq, float gain = 1.2f);
    
    // Bass boost
    void applyBassBoost(float* buffer, size_t samples, float gain = 1.5f);
    
    // Treble adjustment
    void applyTrebleAdjust(float* buffer, size_t samples, float gain = 1.0f);
    
    // Biquad filter
    void applyBiquad(float* buffer, size_t samples, 
                    const BiquadCoeffs& coeffs, BiquadState& state);
    
    // Filter design helpers
    static BiquadCoeffs designLowpass(float freq, float sampleRate, float Q = 0.707f);
    static BiquadCoeffs designHighpass(float freq, float sampleRate, float Q = 0.707f);
    static BiquadCoeffs designPeakingEQ(float freq, float sampleRate, 
                                       float Q, float gainDB);
    
private:
    // Temporary buffers
    float* m_tempBuffer;
    size_t m_tempBufferSize;
    
    // Helper functions
    void ensureTempBuffer(size_t samples);
    float cubicInterpolate(float p0, float p1, float p2, float p3, float t);
};

// ═══════════════════════════════════════════════════════════════════════════
// Helper Functions (use std::lerp from C++20)
// ═══════════════════════════════════════════════════════════════════════════

inline float clamp(float value, float min, float max) {
    return (value < min) ? min : (value > max) ? max : value;
}

#endif // SAM_DSP_PROCESSOR_H