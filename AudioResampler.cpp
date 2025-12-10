/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO RESAMPLER - Implementation                                           ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioResampler.h"

AudioResampler::AudioResampler() 
  : inputRate(0), outputRate(0), ratio(1.0f), 
    quality(RESAMPLE_BEST), phase(0.0f), lastSample(0) {}

void AudioResampler::init(uint32_t inRate, uint32_t outRate, ResampleQuality qual) {
  inputRate = inRate;
  outputRate = outRate;
  quality = qual;
  ratio = (float)inputRate / (float)outputRate;
  phase = 0.0f;
  lastSample = 0;
  
  Serial.printf("[RESAMPLE] %d Hz → %d Hz (ratio: %.3f, quality: %s)\n", 
                inputRate, outputRate, ratio, getQualityName());
}

bool AudioResampler::needsResampling(uint32_t inRate, uint32_t outRate) {
  return (inRate != outRate);
}

const char* AudioResampler::getQualityName() {
  switch(quality) {
    case RESAMPLE_NONE: return "none";
    case RESAMPLE_FAST: return "fast";
    case RESAMPLE_MEDIUM: return "medium";
    case RESAMPLE_HIGH: return "high";
    case RESAMPLE_BEST: return "best";
    default: return "unknown";
  }
}

int16_t AudioResampler::resample(int16_t* inputBuffer, size_t inputSize, size_t& inputPos) {
  if (quality == RESAMPLE_NONE) {
    return resampleNone(inputBuffer[inputPos]);
  }
  
  int16_t sample = 0;
  
  switch(quality) {
    case RESAMPLE_FAST:
      sample = resampleLinear(inputBuffer, phase);
      break;
    case RESAMPLE_MEDIUM:
    case RESAMPLE_HIGH:
      sample = resampleCubic(inputBuffer, phase);
      break;
    case RESAMPLE_BEST:
      sample = resampleSinc(inputBuffer, phase);
      break;
    default:
      sample = inputBuffer[(int)phase];
  }
  
  phase += ratio;
  
  while (phase >= 1.0f) {
    phase -= 1.0f;
    inputPos++;
    if (inputPos >= inputSize) break;
  }
  
  return sample;
}

size_t AudioResampler::resampleBuffer(int16_t* input, size_t inputSamples, 
                                       int16_t* output, size_t outputSamples) {
  size_t inputPos = 0;
  size_t outputPos = 0;
  phase = 0.0f;
  
  while (outputPos < outputSamples && inputPos < inputSamples) {
    output[outputPos++] = resample(input, inputSamples, inputPos);
  }
  
  return outputPos;
}

int16_t AudioResampler::resampleNone(int16_t sample) {
  lastSample = sample;
  return sample;
}

int16_t AudioResampler::resampleLinear(int16_t* buffer, float pos) {
  // Linear interpolation between samples
  int idx = (int)pos;
  float frac = pos - idx;
  
  int16_t s0 = buffer[idx];
  int16_t s1 = buffer[min(idx + 1, (int)inputRate - 1)];
  
  return (int16_t)(s0 + frac * (s1 - s0));
}

int16_t AudioResampler::resampleCubic(int16_t* buffer, float pos) {
  // Cubic interpolation (Hermite spline)
  int idx = (int)pos;
  float frac = pos - idx;
  
  // Need 4 points for cubic
  int16_t s0 = buffer[max(idx - 1, 0)];
  int16_t s1 = buffer[idx];
  int16_t s2 = buffer[min(idx + 1, (int)inputRate - 1)];
  int16_t s3 = buffer[min(idx + 2, (int)inputRate - 1)];
  
  // Hermite interpolation
  float a0 = -0.5f * s0 + 1.5f * s1 - 1.5f * s2 + 0.5f * s3;
  float a1 = s0 - 2.5f * s1 + 2.0f * s2 - 0.5f * s3;
  float a2 = -0.5f * s0 + 0.5f * s2;
  float a3 = s1;
  
  float result = ((a0 * frac + a1) * frac + a2) * frac + a3;
  
  return (int16_t)constrain(result, -32768, 32767);
}

int16_t AudioResampler::resampleSinc(int16_t* buffer, float pos) {
  // Simplified sinc interpolation (Lanczos)
  // For production: use lookup table
  
  int idx = (int)pos;
  float frac = pos - idx;
  
  const int windowSize = 4;
  float sum = 0.0f;
  float weightSum = 0.0f;
  
  for (int i = -windowSize; i <= windowSize; i++) {
    int sampleIdx = idx + i;
    if (sampleIdx < 0 || sampleIdx >= (int)inputRate) continue;
    
    float x = frac - i;
    float weight;
    
    if (fabs(x) < 0.001f) {
      weight = 1.0f;
    } else {
      float pix = PI * x;
      weight = (sin(pix) / pix) * (sin(pix / windowSize) / (pix / windowSize));
    }
    
    sum += buffer[sampleIdx] * weight;
    weightSum += weight;
  }
  
  if (weightSum > 0.0f) {
    sum /= weightSum;
  }
  
  return (int16_t)constrain(sum, -32768, 32767);
}
