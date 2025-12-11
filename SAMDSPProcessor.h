// SAMDSPProcessor.h - DSP Enhancement for SAM Speech
// Advanced signal processing inspired by the HTML implementation
// ESP32-optimized using FPU and SIMD where possible

#ifndef SAM_DSP_PROCESSOR_H
#define SAM_DSP_PROCESSOR_H

#include <Arduino.h>

// ============================================================================
// DSP Processor Class
// ============================================================================

class SAMDSPProcessor {
public:
    SAMDSPProcessor();
    ~SAMDSPProcessor();
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    void begin(uint32_t sampleRate = 22050);
    
    // ========================================================================
    // DSP Enhancement Functions
    // ========================================================================
    
    // Moving average smoothing (reduces digital harshness)
    void applySmoothing(float* buffer, size_t length, float amount);
    
    // Cubic interpolation (improves formant transitions)
    void applyCubicInterpolation(float* buffer, size_t length, float amount);
    
    // Formant boost (enhances intelligibility)
    void applyFormantBoost(float* buffer, size_t length, float amount);
    
    // Bass/treble EQ
    void applyBassBoost(float* buffer, size_t length, float dB);
    void applyTrebleBoost(float* buffer, size_t length, float dB);
    
    // Comprehensive processing pipeline
    void processBuffer(float* buffer, size_t length, 
                      float smoothAmount, float interpAmount,
                      float formantAmount, float bassdB);
    
    // ========================================================================
    // Advanced DSP (for future enhancements)
    // ========================================================================
    
    // Noise reduction
    void applyNoiseGate(float* buffer, size_t length, float threshold);
    
    // Dynamic range compression
    void applyCompression(float* buffer, size_t length, 
                         float threshold, float ratio);
    
    // Reverb/Echo effects
    void applyReverb(float* buffer, size_t length, 
                    float roomSize, float damping);
    
private:
    // ========================================================================
    // Internal DSP Functions
    // ========================================================================
    
    // Cubic interpolation helper
    static float cubicInterpolate(float p0, float p1, float p2, float p3, float t);
    
    // Biquad filter implementation (for EQ)
    struct BiquadFilter {
        float b0, b1, b2, a1, a2;
        float x1, x2, y1, y2;
        
        BiquadFilter() : b0(1), b1(0), b2(0), a1(0), a2(0),
                         x1(0), x2(0), y1(0), y2(0) {}
        
        void reset() { x1 = x2 = y1 = y2 = 0; }
        
        float process(float input) {
            float output = b0 * input + b1 * x1 + b2 * x2 
                          - a1 * y1 - a2 * y2;
            x2 = x1; x1 = input;
            y2 = y1; y1 = output;
            return output;
        }
    };
    
    // Design low-shelf filter
    void designLowShelf(BiquadFilter& filter, float freq, float gain, float Q = 0.707f);
    
    // Design high-shelf filter
    void designHighShelf(BiquadFilter& filter, float freq, float gain, float Q = 0.707f);
    
    // Design peaking filter
    void designPeaking(BiquadFilter& filter, float freq, float gain, float Q);
    
    // ========================================================================
    // Member Variables
    // ========================================================================
    
    uint32_t m_sampleRate;
    
    // Filters for EQ
    BiquadFilter m_bassFilter;
    BiquadFilter m_trebleFilter;
    BiquadFilter m_formantFilter1;  // ~500 Hz
    BiquadFilter m_formantFilter2;  // ~1500 Hz
    BiquadFilter m_formantFilter3;  // ~2500 Hz
    
    // Reverb state
    static constexpr size_t MAX_REVERB_DELAY = 8192;
    float* m_reverbBuffer;
    size_t m_reverbPos;
    
    // Compressor state
    float m_compressorEnvelope;
    static constexpr float COMPRESSOR_ATTACK = 0.001f;
    static constexpr float COMPRESSOR_RELEASE = 0.1f;
};

// ============================================================================
// Inline Helper Functions (FPU-optimized)
// ============================================================================

// Fast sin approximation (faster than standard sin for speech synthesis)
inline float fastSin(float x) {
    // Bhaskara I's sine approximation
    const float B = 4.0f / PI;
    const float C = -4.0f / (PI * PI);
    const float P = 0.225f;
    
    x = fmodf(x, 2.0f * PI);
    if (x < 0) x += 2.0f * PI;
    
    float y = B * x + C * x * fabsf(x);
    y = P * (y * fabsf(y) - y) + y;
    
    return y;
}

// Fast clamp (branch-free)
inline float clamp(float value, float min, float max) {
    return fminf(fmaxf(value, min), max);
}

// Fast linear interpolation (use std::lerp from C++20)
// inline float lerp(float a, float b, float t) {
//     return a + t * (b - a);
// }

#endif // SAM_DSP_PROCESSOR_H