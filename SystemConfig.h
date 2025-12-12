// SystemConfig.h - Runtime System Configuration

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>
#include "AudioConfig.h"

class AudioFilesystem;

struct SystemConfig {
  // Audio Defaults
  uint32_t defaultSampleRate;
  uint8_t defaultMaxVoices;
  uint8_t defaultVolume;
  char defaultAudioMode[8];
  
  // I2S Defaults
  uint8_t i2sDefaultPin;
  uint32_t i2sDefaultBuffer;
  uint32_t i2sDefaultBuffers;
  int16_t i2sDefaultAmplitude;
  
  // PWM Defaults
  uint8_t pwmDefaultPin;
  uint32_t pwmDefaultFreq;
  uint8_t pwmDefaultRes;
  int16_t pwmDefaultAmplitude;
  uint8_t pwmDefaultGain;
  
  // Effects Defaults
  bool defaultEQEnabled;
  bool defaultFilterEnabled;
  bool defaultReverbEnabled;
  bool defaultLFOEnabled;
  bool defaultDelayEnabled;
  
  // Startup
  char startupProfile[MAX_PROFILE_NAME];
  
  // Features
  bool enableAudioEngine;
  bool enableDisplay;
  bool enableWiFi;
  bool enableBluetooth;
  
  SystemConfig();
  bool loadFromFile(AudioFilesystem* fs);
  bool saveToFile(AudioFilesystem* fs);
  void setDefaults();
  
private:
  bool loadFromJSON(const char* path, AudioFilesystem* fs);
  bool saveToJSON(const char* path, AudioFilesystem* fs);
};

#endif // SYSTEM_CONFIG_H