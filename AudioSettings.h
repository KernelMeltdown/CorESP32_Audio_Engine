// AudioSettings.h - ESP32 Audio OS v1.9
// Runtime Configuration with LFO Modulation

#ifndef AUDIO_SETTINGS_H
#define AUDIO_SETTINGS_H

#include <Arduino.h>
#include "AudioConfig.h"

// Forward declaration from AudioEngine.h
enum WaveformType {
  WAVE_SINE,
  WAVE_SQUARE,
  WAVE_SAWTOOTH,
  WAVE_TRIANGLE,
  WAVE_NOISE
};

// Filter types
enum FilterType {
  FILTER_LOWPASS,
  FILTER_HIGHPASS,
  FILTER_BANDPASS
};

// ============================================================================
// ENUMS
// ============================================================================

enum AudioMode {
  MODE_I2S,
  MODE_PWM
};

enum ResampleQuality {
  RESAMPLE_NONE,
  RESAMPLE_FAST,
  RESAMPLE_MEDIUM,
  RESAMPLE_HIGH,
  RESAMPLE_BEST
};

// ============================================================================
// MULTI-CORE CONFIGURATION
// ============================================================================
struct MultiCoreConfig {
  bool useDualCore;
  uint8_t audioCore;
  uint8_t uiCore;
  bool useLPCore;
  
  MultiCoreConfig() {
    #if HAS_DUAL_CORE
      useDualCore = true;
      audioCore = DEFAULT_AUDIO_CORE;
      uiCore = DEFAULT_UI_CORE;
    #else
      useDualCore = false;
      audioCore = 0;
      uiCore = 0;
    #endif
    
    #if HAS_LP_CORE
      useLPCore = false;
    #else
      useLPCore = false;
    #endif
  }
};

// ============================================================================
// PERFORMANCE CONFIGURATION
// ============================================================================
struct PerformanceConfig {
  bool useFixedPoint;
  bool useWavetable;
  bool enableCPUMonitor;
  uint32_t i2sBufferSize;
  uint32_t i2sNumBuffers;
  
  PerformanceConfig() {
    useFixedPoint = USE_FIXED_POINT_MATH;
    useWavetable = USE_WAVETABLE_LOOKUP;
    enableCPUMonitor = ENABLE_CPU_MONITOR;
    i2sBufferSize = DEFAULT_I2S_BUFFER;
    i2sNumBuffers = DEFAULT_I2S_BUFFERS;
  }
};

// ============================================================================
// I2S CONFIGURATION
// ============================================================================
struct I2SConfig {
  uint8_t pin;
  uint32_t bufferSize;
  uint32_t numBuffers;
  int16_t amplitude;
  
  I2SConfig() {
    pin = DEFAULT_I2S_PIN;
    bufferSize = DEFAULT_I2S_BUFFER;
    numBuffers = DEFAULT_I2S_BUFFERS;
    amplitude = DEFAULT_I2S_AMPLITUDE;
  }
};

// ============================================================================
// PWM CONFIGURATION
// ============================================================================
struct PWMConfig {
  uint8_t pin;
  uint32_t frequency;
  uint8_t resolution;
  int16_t amplitude;
  uint8_t gain;
  
  PWMConfig() {
    pin = DEFAULT_PWM_PIN;
    frequency = DEFAULT_PWM_FREQUENCY;
    resolution = DEFAULT_PWM_RESOLUTION;
    amplitude = DEFAULT_PWM_AMPLITUDE;
    gain = DEFAULT_PWM_GAIN;
  }
};

// ============================================================================
// EQ CONFIGURATION (BIQUAD)
// ============================================================================
struct EQConfig {
  bool enabled;
  int8_t bass;
  int8_t mid;
  int8_t treble;
  
  uint16_t bassFreq;
  uint16_t midFreq;
  uint16_t trebleFreq;
  float q;
  
  EQConfig() {
    enabled = DEFAULT_EQ_ENABLED;
    bass = DEFAULT_EQ_BASS;
    mid = DEFAULT_EQ_MID;
    treble = DEFAULT_EQ_TREBLE;
    bassFreq = EQ_BASS_FREQ;
    midFreq = EQ_MID_FREQ;
    trebleFreq = EQ_TREBLE_FREQ;
    q = EQ_Q_FACTOR;
  }
};

// ============================================================================
// STATE-VARIABLE FILTER CONFIGURATION
// ============================================================================
struct FilterConfig {
  bool enabled;
  FilterType type;
  float cutoff;
  float resonance;
  
  FilterConfig() {
    enabled = DEFAULT_FILTER_ENABLED;
    type = (FilterType)DEFAULT_FILTER_TYPE;
    cutoff = DEFAULT_FILTER_CUTOFF;
    resonance = DEFAULT_FILTER_RESONANCE;
  }
  
  const char* getTypeName() const {
    switch(type) {
      case FILTER_LOWPASS: return "Lowpass";
      case FILTER_HIGHPASS: return "Highpass";
      case FILTER_BANDPASS: return "Bandpass";
      default: return "Unknown";
    }
  }
  
  void setType(const char* typeName) {
    if (strcmp(typeName, "lowpass") == 0 || strcmp(typeName, "lp") == 0) {
      type = FILTER_LOWPASS;
    } else if (strcmp(typeName, "highpass") == 0 || strcmp(typeName, "hp") == 0) {
      type = FILTER_HIGHPASS;
    } else if (strcmp(typeName, "bandpass") == 0 || strcmp(typeName, "bp") == 0) {
      type = FILTER_BANDPASS;
    }
  }
};

// ============================================================================
// SCHROEDER REVERB CONFIGURATION
// ============================================================================
struct ReverbConfig {
  bool enabled;
  float roomSize;
  float damping;
  float wet;
  
  ReverbConfig() {
    enabled = DEFAULT_REVERB_ENABLED;
    roomSize = DEFAULT_REVERB_ROOM_SIZE;
    damping = DEFAULT_REVERB_DAMPING;
    wet = DEFAULT_REVERB_WET;
  }
};

// ============================================================================
// LFO MODULATION CONFIGURATION (NEW!)
// ============================================================================
struct LFOConfig {
  bool enabled;
  bool vibratoEnabled;
  bool tremoloEnabled;
  float rate;     // Hz (0.1 - 20 Hz)
  float depth;    // % (0 - 100%)
  
  LFOConfig() {
    enabled = DEFAULT_LFO_ENABLED;
    vibratoEnabled = DEFAULT_LFO_VIBRATO_ENABLED;
    tremoloEnabled = DEFAULT_LFO_TREMOLO_ENABLED;
    rate = DEFAULT_LFO_RATE;
    depth = DEFAULT_LFO_DEPTH;
  }
};

// ============================================================================
// DELAY CONFIGURATION
// ============================================================================
struct DelayConfig {
  bool enabled;
  uint16_t timeMs;
  uint8_t feedback;
  uint8_t mix;
  
  DelayConfig() {
    enabled = DEFAULT_DELAY_ENABLED;
    timeMs = DEFAULT_DELAY_TIME;
    feedback = DEFAULT_DELAY_FEEDBACK;
    mix = DEFAULT_DELAY_MIX;
  }
};

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================
struct DisplayConfig {
  bool enabled;
  uint8_t brightness;
  
  DisplayConfig() {
    enabled = DISPLAY_ENABLED;
    #if DISPLAY_ENABLED
      brightness = DEFAULT_BRIGHTNESS;
    #else
      brightness = 0;
    #endif
  }
};

// ============================================================================
// MAIN SETTINGS STRUCTURE
// ============================================================================
struct AudioSettings {
  char name[MAX_PROFILE_NAME];
  char description[MAX_PROFILE_DESC];
  
  AudioMode mode;
  uint32_t sampleRate;
  uint8_t voices;
  uint8_t volume;
  
  WaveformType waveform;
  
  I2SConfig i2s;
  PWMConfig pwm;
  
  EQConfig eq;
  FilterConfig filter;
  ReverbConfig reverb;
  LFOConfig lfo;        // NEW!
  DelayConfig delay;
  
  ResampleQuality resampleQuality;
  DisplayConfig display;
  MultiCoreConfig multiCore;
  PerformanceConfig performance;
  
  AudioSettings() {
    strncpy(name, "default", MAX_PROFILE_NAME);
    strncpy(description, "Factory default settings", MAX_PROFILE_DESC);
    
    mode = MODE_I2S;
    sampleRate = DEFAULT_SAMPLE_RATE;
    voices = DEFAULT_MAX_VOICES;
    volume = DEFAULT_VOLUME;
    
    waveform = WAVE_SINE;
    resampleQuality = RESAMPLE_BEST;
  }
  
  const char* getModeName() const {
    return (mode == MODE_I2S) ? "i2s" : "pwm";
  }
  
  void setMode(const char* modeName) {
    if (strcmp(modeName, "i2s") == 0) {
      mode = MODE_I2S;
    } else if (strcmp(modeName, "pwm") == 0) {
      mode = MODE_PWM;
    }
  }
  
  const char* getResampleQualityName() const {
    switch(resampleQuality) {
      case RESAMPLE_NONE: return "none";
      case RESAMPLE_FAST: return "fast";
      case RESAMPLE_MEDIUM: return "medium";
      case RESAMPLE_HIGH: return "high";
      case RESAMPLE_BEST: return "best";
      default: return "unknown";
    }
  }
  
  void setResampleQuality(const char* qualityName) {
    if (strcmp(qualityName, "none") == 0) resampleQuality = RESAMPLE_NONE;
    else if (strcmp(qualityName, "fast") == 0) resampleQuality = RESAMPLE_FAST;
    else if (strcmp(qualityName, "medium") == 0) resampleQuality = RESAMPLE_MEDIUM;
    else if (strcmp(qualityName, "high") == 0) resampleQuality = RESAMPLE_HIGH;
    else if (strcmp(qualityName, "best") == 0) resampleQuality = RESAMPLE_BEST;
  }
  
  const char* getWaveformName() const {
    switch(waveform) {
      case WAVE_SINE: return "sine";
      case WAVE_SQUARE: return "square";
      case WAVE_SAWTOOTH: return "sawtooth";
      case WAVE_TRIANGLE: return "triangle";
      case WAVE_NOISE: return "noise";
      default: return "unknown";
    }
  }
  
  void setWaveform(const char* waveformName) {
    if (strcmp(waveformName, "sine") == 0) waveform = WAVE_SINE;
    else if (strcmp(waveformName, "square") == 0) waveform = WAVE_SQUARE;
    else if (strcmp(waveformName, "sawtooth") == 0 || strcmp(waveformName, "saw") == 0) waveform = WAVE_SAWTOOTH;
    else if (strcmp(waveformName, "triangle") == 0 || strcmp(waveformName, "tri") == 0) waveform = WAVE_TRIANGLE;
    else if (strcmp(waveformName, "noise") == 0) waveform = WAVE_NOISE;
  }
};

#endif // AUDIO_SETTINGS_H
