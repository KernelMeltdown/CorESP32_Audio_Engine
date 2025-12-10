/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO RESAMPLER - Sample Rate Conversion Engine                            ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#ifndef AUDIO_RESAMPLER_H
#define AUDIO_RESAMPLER_H

#include <Arduino.h>
#include "AudioConfig.h"
#include "AudioSettings.h"

class AudioResampler {
public:
  AudioResampler();
  
  // Setup
  void init(uint32_t inputRate, uint32_t outputRate, ResampleQuality quality);
  
  // Check if resampling needed
  bool needsResampling(uint32_t inputRate, uint32_t outputRate);
  
  // Resample single sample
  int16_t resample(int16_t* inputBuffer, size_t inputSize, size_t& inputPos);
  
  // Resample buffer
  size_t resampleBuffer(int16_t* input, size_t inputSamples, 
                        int16_t* output, size_t outputSamples);
  
  // Info
  float getRatio() { return ratio; }
  const char* getQualityName();
  
private:
  uint32_t inputRate;
  uint32_t outputRate;
  float ratio;
  ResampleQuality quality;
  
  float phase;
  int16_t lastSample;
  
  // Resampling methods
  int16_t resampleNone(int16_t sample);
  int16_t resampleLinear(int16_t* buffer, float pos);
  int16_t resampleCubic(int16_t* buffer, float pos);
  int16_t resampleSinc(int16_t* buffer, float pos);
};

#endif // AUDIO_RESAMPLER_H
