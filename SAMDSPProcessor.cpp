/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM DSP PROCESSOR - Implementation                                         ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#include "SAMDSPProcessor.h"
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════

SAMDSPProcessor::SAMDSPProcessor() 
    : m_tempBuffer(nullptr)
    , m_tempBufferSize(0)
{
}

SAMDSPProcessor::~SAMDSPProcessor() {
    if (m_tempBuffer) {
        delete[] m_tempBuffer;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Buffer Management
// ═══════════════════════════════════════════════════════════════════════════

void SAMDSPProcessor::ensureTempBuffer(size_t samples) {
    if (samples > m_tempBufferSize) {
        if (m_tempBuffer) delete[] m_tempBuffer;
        m_tempBuffer = new float[samples];
        m_tempBufferSize = samples;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Smoothing
// ═══════════════════════════════════════════════════════════════════════════

void SAMDSPProcessor::applySmoothing(float* buffer, size_t samples, float amount) {
    if (!buffer || samples < 2) return;
    
    ensureTempBuffer(samples);
    memcpy(m_tempBuffer, buffer, samples * sizeof(float));
    
    // Simple box filter with variable amount
    for (size_t i = 1; i < samples - 1; i++) {
        float smoothed = (m_tempBuffer[i-1] + m_tempBuffer[i] + m_tempBuffer[i+1]) / 3.0f;
        buffer[i] = std::lerp(m_tempBuffer[i], smoothed, amount);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Cubic Interpolation
// ═══════════════════════════════════════════════════════════════════════════

float SAMDSPProcessor::cubicInterpolate(float p0, float p1, float p2, float p3, float t) {
    float a = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
    float b = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
    float c = -0.5f * p0 + 0.5f * p2;
    float d = p1;
    
    return a * t * t * t + b * t * t + c * t + d;
}

void SAMDSPProcessor::applyCubicInterpolation(float* buffer, size_t samples, float amount) {
    if (!buffer || samples < 4) return;
    
    ensureTempBuffer(samples);
    memcpy(m_tempBuffer, buffer, samples * sizeof(float));
    
    for (size_t i = 2; i < samples - 2; i++) {
        float interpolated = cubicInterpolate(
            m_tempBuffer[i-2],
            m_tempBuffer[i-1],
            m_tempBuffer[i],
            m_tempBuffer[i+1],
            0.5f
        );
        buffer[i] = std::lerp(m_tempBuffer[i], interpolated, amount);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Formant Boost
// ═══════════════════════════════════════════════════════════════════════════

void SAMDSPProcessor::applyFormantBoost(float* buffer, size_t samples, 
                                       float freq, float gain) {
    if (!buffer || samples == 0) return;
    
    // Design peaking EQ filter for formant boost
    BiquadCoeffs coeffs = designPeakingEQ(freq, 22050.0f, 2.0f, 
                                         20.0f * std::log10(gain));
    BiquadState state;
    
    applyBiquad(buffer, samples, coeffs, state);
}

// ═══════════════════════════════════════════════════════════════════════════
// Bass/Treble
// ═══════════════════════════════════════════════════════════════════════════

void SAMDSPProcessor::applyBassBoost(float* buffer, size_t samples, float gain) {
    if (!buffer || samples == 0) return;
    
    // Low shelf filter at 200 Hz
    BiquadCoeffs coeffs = designPeakingEQ(200.0f, 22050.0f, 1.0f, 
                                         20.0f * std::log10(gain));
    BiquadState state;
    
    applyBiquad(buffer, samples, coeffs, state);
}

void SAMDSPProcessor::applyTrebleAdjust(float* buffer, size_t samples, float gain) {
    if (!buffer || samples == 0) return;
    
    // High shelf filter at 4000 Hz
    BiquadCoeffs coeffs = designPeakingEQ(4000.0f, 22050.0f, 1.0f, 
                                         20.0f * std::log10(gain));
    BiquadState state;
    
    applyBiquad(buffer, samples, coeffs, state);
}

// ═══════════════════════════════════════════════════════════════════════════
// Biquad Filter
// ═══════════════════════════════════════════════════════════════════════════

void SAMDSPProcessor::applyBiquad(float* buffer, size_t samples,
                                 const BiquadCoeffs& coeffs, BiquadState& state) {
    if (!buffer || samples == 0) return;
    
    for (size_t i = 0; i < samples; i++) {
        float x = buffer[i];
        float y = coeffs.b0 * x + coeffs.b1 * state.x1 + coeffs.b2 * state.x2
                - coeffs.a1 * state.y1 - coeffs.a2 * state.y2;
        
        state.x2 = state.x1;
        state.x1 = x;
        state.y2 = state.y1;
        state.y1 = y;
        
        buffer[i] = y;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Filter Design
// ═══════════════════════════════════════════════════════════════════════════

BiquadCoeffs SAMDSPProcessor::designLowpass(float freq, float sampleRate, float Q) {
    BiquadCoeffs coeffs;
    
    float w0 = 2.0f * M_PI * freq / sampleRate;
    float alpha = std::sin(w0) / (2.0f * Q);
    float cosw0 = std::cos(w0);
    
    float a0 = 1.0f + alpha;
    coeffs.b0 = (1.0f - cosw0) / (2.0f * a0);
    coeffs.b1 = (1.0f - cosw0) / a0;
    coeffs.b2 = (1.0f - cosw0) / (2.0f * a0);
    coeffs.a1 = (-2.0f * cosw0) / a0;
    coeffs.a2 = (1.0f - alpha) / a0;
    
    return coeffs;
}

BiquadCoeffs SAMDSPProcessor::designHighpass(float freq, float sampleRate, float Q) {
    BiquadCoeffs coeffs;
    
    float w0 = 2.0f * M_PI * freq / sampleRate;
    float alpha = std::sin(w0) / (2.0f * Q);
    float cosw0 = std::cos(w0);
    
    float a0 = 1.0f + alpha;
    coeffs.b0 = (1.0f + cosw0) / (2.0f * a0);
    coeffs.b1 = -(1.0f + cosw0) / a0;
    coeffs.b2 = (1.0f + cosw0) / (2.0f * a0);
    coeffs.a1 = (-2.0f * cosw0) / a0;
    coeffs.a2 = (1.0f - alpha) / a0;
    
    return coeffs;
}

BiquadCoeffs SAMDSPProcessor::designPeakingEQ(float freq, float sampleRate, 
                                              float Q, float gainDB) {
    BiquadCoeffs coeffs;
    
    float A = std::pow(10.0f, gainDB / 40.0f);
    float w0 = 2.0f * M_PI * freq / sampleRate;
    float alpha = std::sin(w0) / (2.0f * Q);
    float cosw0 = std::cos(w0);
    
    float a0 = 1.0f + alpha / A;
    coeffs.b0 = (1.0f + alpha * A) / a0;
    coeffs.b1 = (-2.0f * cosw0) / a0;
    coeffs.b2 = (1.0f - alpha * A) / a0;
    coeffs.a1 = (-2.0f * cosw0) / a0;
    coeffs.a2 = (1.0f - alpha / A) / a0;
    
    return coeffs;
}