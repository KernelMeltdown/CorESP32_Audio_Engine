// SAMDSPProcessor.cpp - Audio Enhancement Implementation
// ESP32-optimized DSP processing

#include "SAMDSPProcessor.h"
#include <Arduino.h>

// ============================================================================
// Constructor & Destructor
// ============================================================================

SAMDSPProcessor::SAMDSPProcessor()
    : m_sampleRate(22050)
    , m_reverbBuffer(nullptr)
    , m_reverbPos(0)
    , m_compressorEnvelope(0)
{
}

SAMDSPProcessor::~SAMDSPProcessor() {
    if (m_reverbBuffer) {
        free(m_reverbBuffer);
    }
}

// ============================================================================
// Initialization
// ============================================================================

void SAMDSPProcessor::begin(uint32_t sampleRate) {
    m_sampleRate = sampleRate;
    
    // Initialize filters
    m_bassFilter.reset();
    m_trebleFilter.reset();
    m_formantFilter1.reset();
    m_formantFilter2.reset();
    m_formantFilter3.reset();
    
    // Design default EQ filters
    designLowShelf(m_bassFilter, 200.0f, 0.0f);
    designHighShelf(m_trebleFilter, 4000.0f, 0.0f);
    designPeaking(m_formantFilter1, 500.0f, 0.0f, 2.0f);
    designPeaking(m_formantFilter2, 1500.0f, 0.0f, 2.0f);
    designPeaking(m_formantFilter3, 2500.0f, 0.0f, 2.0f);
    
    // Allocate reverb buffer
    if (!m_reverbBuffer) {
        m_reverbBuffer = (float*)calloc(MAX_REVERB_DELAY, sizeof(float));
    }
}

// ============================================================================
// Main Processing Pipeline
// ============================================================================

void SAMDSPProcessor::processBuffer(float* buffer, size_t length, 
                                   float smoothAmount, float interpAmount,
                                   float formantAmount, float bassdB) {
    if (!buffer || length == 0) return;
    
    // 1. Smoothing (reduces digital artifacts)
    if (smoothAmount > 0.01f) {
        applySmoothing(buffer, length, smoothAmount);
    }
    
    // 2. Cubic interpolation (smooths formant transitions)
    if (interpAmount > 0.01f) {
        applyCubicInterpolation(buffer, length, interpAmount);
    }
    
    // 3. Formant boost (enhances intelligibility)
    if (formantAmount > 0.01f) {
        applyFormantBoost(buffer, length, formantAmount);
    }
    
    // 4. Bass EQ
    if (fabs(bassdB) > 0.1f) {
        applyBassBoost(buffer, length, bassdB);
    }
}

// ============================================================================
// Smoothing (Moving Average)
// ============================================================================

void SAMDSPProcessor::applySmoothing(float* buffer, size_t length, float amount) {
    if (amount <= 0 || length < 5) return;
    
    // Weighted moving average with 9-point window
    const float weights[] = {0.05f, 0.1f, 0.15f, 0.2f, 0.4f, 0.2f, 0.15f, 0.1f, 0.05f};
    const int halfWindow = 4;
    
    // Create temporary buffer
    float* temp = (float*)malloc(length * sizeof(float));
    if (!temp) return;
    
    memcpy(temp, buffer, length * sizeof(float));
    
    // Apply weighted average
    for (size_t i = halfWindow; i < length - halfWindow; i++) {
        float smoothed = 0;
        
        for (int j = -halfWindow; j <= halfWindow; j++) {
            smoothed += temp[i + j] * weights[j + halfWindow];
        }
        
        // Blend with original
        buffer[i] = std::lerp(temp[i], smoothed, amount);
    }
    
    free(temp);
}

// ============================================================================
// Cubic Interpolation
// ============================================================================

float SAMDSPProcessor::cubicInterpolate(float p0, float p1, float p2, float p3, float t) {
    // Catmull-Rom cubic interpolation
    float a0 = p3 - p2 - p0 + p1;
    float a1 = p0 - p1 - a0;
    float a2 = p2 - p0;
    float a3 = p1;
    
    return a0 * t * t * t + a1 * t * t + a2 * t + a3;
}

void SAMDSPProcessor::applyCubicInterpolation(float* buffer, size_t length, float amount) {
    if (amount <= 0 || length < 4) return;
    
    float* temp = (float*)malloc(length * sizeof(float));
    if (!temp) return;
    
    memcpy(temp, buffer, length * sizeof(float));
    
    // Apply cubic interpolation
    for (size_t i = 2; i < length - 2; i++) {
        float p0 = temp[i - 2];
        float p1 = temp[i - 1];
        float p2 = temp[i];
        float p3 = temp[i + 1];
        
        float interpolated = cubicInterpolate(p0, p1, p2, p3, 0.5f);
        
        // Blend with original
        buffer[i] = std::lerp(temp[i], interpolated, amount);
    }
    
    free(temp);
}

// ============================================================================
// Formant Boost (Enhances Speech Intelligibility)
// ============================================================================

void SAMDSPProcessor::applyFormantBoost(float* buffer, size_t length, float amount) {
    if (amount <= 0) return;
    
    // Boost signal amplitude in formant regions
    float boostFactor = 1.0f + amount * 0.3f;
    
    for (size_t i = 0; i < length; i++) {
        float sample = buffer[i];
        
        // Enhance amplitude
        float boosted = sample * boostFactor;
        
        // Soft clipping to prevent excessive peaks
        if (boosted > 1.0f) {
            boosted = 1.0f - expf(-(boosted - 1.0f));
        } else if (boosted < -1.0f) {
            boosted = -1.0f + expf(-(fabs(boosted) - 1.0f));
        }
        
        buffer[i] = boosted;
    }
}

// ============================================================================
// Bass Boost/Cut
// ============================================================================

void SAMDSPProcessor::applyBassBoost(float* buffer, size_t length, float dB) {
    if (fabs(dB) < 0.1f) return;
    
    // Redesign bass filter with new gain
    designLowShelf(m_bassFilter, 200.0f, dB);
    
    // Apply filter
    for (size_t i = 0; i < length; i++) {
        buffer[i] = m_bassFilter.process(buffer[i]);
    }
}

void SAMDSPProcessor::applyTrebleBoost(float* buffer, size_t length, float dB) {
    if (fabs(dB) < 0.1f) return;
    
    designHighShelf(m_trebleFilter, 4000.0f, dB);
    
    for (size_t i = 0; i < length; i++) {
        buffer[i] = m_trebleFilter.process(buffer[i]);
    }
}

// ============================================================================
// Biquad Filter Design
// ============================================================================

void SAMDSPProcessor::designLowShelf(BiquadFilter& filter, float freq, float gain, float Q) {
    float A = powf(10.0f, gain / 40.0f);
    float w0 = 2.0f * PI * freq / m_sampleRate;
    float cosw0 = cosf(w0);
    float sinw0 = sinf(w0);
    float alpha = sinw0 / (2.0f * Q);
    
    float b0 = A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * sqrtf(A) * alpha);
    float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0);
    float b2 = A * ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * sqrtf(A) * alpha);
    float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * sqrtf(A) * alpha;
    float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0);
    float a2 = (A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * sqrtf(A) * alpha;
    
    filter.b0 = b0 / a0;
    filter.b1 = b1 / a0;
    filter.b2 = b2 / a0;
    filter.a1 = a1 / a0;
    filter.a2 = a2 / a0;
    filter.reset();
}

void SAMDSPProcessor::designHighShelf(BiquadFilter& filter, float freq, float gain, float Q) {
    float A = powf(10.0f, gain / 40.0f);
    float w0 = 2.0f * PI * freq / m_sampleRate;
    float cosw0 = cosf(w0);
    float sinw0 = sinf(w0);
    float alpha = sinw0 / (2.0f * Q);
    
    float b0 = A * ((A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * sqrtf(A) * alpha);
    float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0);
    float b2 = A * ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * sqrtf(A) * alpha);
    float a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * sqrtf(A) * alpha;
    float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0);
    float a2 = (A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * sqrtf(A) * alpha;
    
    filter.b0 = b0 / a0;
    filter.b1 = b1 / a0;
    filter.b2 = b2 / a0;
    filter.a1 = a1 / a0;
    filter.a2 = a2 / a0;
    filter.reset();
}

void SAMDSPProcessor::designPeaking(BiquadFilter& filter, float freq, float gain, float Q) {
    float A = powf(10.0f, gain / 40.0f);
    float w0 = 2.0f * PI * freq / m_sampleRate;
    float cosw0 = cosf(w0);
    float sinw0 = sinf(w0);
    float alpha = sinw0 / (2.0f * Q);
    
    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cosw0;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cosw0;
    float a2 = 1.0f - alpha / A;
    
    filter.b0 = b0 / a0;
    filter.b1 = b1 / a0;
    filter.b2 = b2 / a0;
    filter.a1 = a1 / a0;
    filter.a2 = a2 / a0;
    filter.reset();
}

// ============================================================================
// Advanced Effects (for future use)
// ============================================================================

void SAMDSPProcessor::applyNoiseGate(float* buffer, size_t length, float threshold) {
    for (size_t i = 0; i < length; i++) {
        if (fabsf(buffer[i]) < threshold) {
            buffer[i] = 0;
        }
    }
}

void SAMDSPProcessor::applyCompression(float* buffer, size_t length, 
                                      float threshold, float ratio) {
    for (size_t i = 0; i < length; i++) {
        float sample = buffer[i];
        float amplitude = fabsf(sample);
        
        // Envelope follower
        if (amplitude > m_compressorEnvelope) {
            m_compressorEnvelope += (amplitude - m_compressorEnvelope) * COMPRESSOR_ATTACK;
        } else {
            m_compressorEnvelope += (amplitude - m_compressorEnvelope) * COMPRESSOR_RELEASE;
        }
        
        // Apply compression above threshold
        if (m_compressorEnvelope > threshold) {
            float excess = m_compressorEnvelope - threshold;
            float reduction = excess * (1.0f - 1.0f / ratio);
            float gain = (threshold + excess - reduction) / m_compressorEnvelope;
            buffer[i] *= gain;
        }
    }
}

void SAMDSPProcessor::applyReverb(float* buffer, size_t length, 
                                 float roomSize, float damping) {
    if (!m_reverbBuffer) return;
    
    size_t delayTime = (size_t)(roomSize * MAX_REVERB_DELAY);
    if (delayTime >= MAX_REVERB_DELAY) delayTime = MAX_REVERB_DELAY - 1;
    
    for (size_t i = 0; i < length; i++) {
        // Get delayed sample
        float delayed = m_reverbBuffer[m_reverbPos];
        
        // Mix with input
        float output = buffer[i] + delayed * damping;
        
        // Store in delay buffer
        m_reverbBuffer[m_reverbPos] = output;
        
        // Advance position
        m_reverbPos = (m_reverbPos + 1) % MAX_REVERB_DELAY;
        
        buffer[i] = output;
    }
}