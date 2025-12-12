// SystemConfig.cpp - Runtime System Configuration

#include "SystemConfig.h"
#include "AudioFilesystem.h"
#include <ArduinoJson.h>

SystemConfig::SystemConfig() {
  setDefaults();
}

void SystemConfig::setDefaults() {
  defaultSampleRate = 22050;
  defaultMaxVoices = 4;
  defaultVolume = 200;
  strncpy(defaultAudioMode, "i2s", sizeof(defaultAudioMode));
  
  i2sDefaultPin = 1;
  i2sDefaultBuffer = 128;
  i2sDefaultBuffers = 4;
  i2sDefaultAmplitude = 12000;
  
  pwmDefaultPin = 2;
  pwmDefaultFreq = 78125;
  pwmDefaultRes = 9;
  pwmDefaultAmplitude = 5000;
  pwmDefaultGain = 7;
  
  defaultEQEnabled = false;
  defaultFilterEnabled = false;
  defaultReverbEnabled = false;
  defaultLFOEnabled = false;
  defaultDelayEnabled = false;
  
  strncpy(startupProfile, "default", sizeof(startupProfile));
  
  enableAudioEngine = true;
  enableDisplay = false;
  enableWiFi = false;
  enableBluetooth = false;
}

bool SystemConfig::loadFromFile(AudioFilesystem* fs) {
  if (!fs || !fs->isInitialized()) {
    Serial.println(F("[SYSCFG] Filesystem not ready, using defaults"));
    return false;
  }
  
  if (!fs->exists(PATH_SYSTEM_CONFIG)) {
    Serial.println(F("[SYSCFG] No system.json found, creating defaults"));
    saveToFile(fs);
    return true;
  }
  
  return loadFromJSON(PATH_SYSTEM_CONFIG, fs);
}

bool SystemConfig::saveToFile(AudioFilesystem* fs) {
  if (!fs || !fs->isInitialized()) {
    return false;
  }
  
  return saveToJSON(PATH_SYSTEM_CONFIG, fs);
}

bool SystemConfig::loadFromJSON(const char* path, AudioFilesystem* fs) {
  File file = fs->open(path, "r");
  if (!file) {
    Serial.println(F("[SYSCFG] Cannot open system.json"));
    return false;
  }
  
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("[SYSCFG] JSON parse error: %s\n", error.c_str());
    return false;
  }
  
  // Parse Audio Defaults
  JsonObject audio = doc["defaults"]["audio"];
  if (!audio.isNull()) {
    defaultSampleRate = audio["sampleRate"] | defaultSampleRate;
    defaultMaxVoices = audio["voices"] | defaultMaxVoices;
    defaultVolume = audio["volume"] | defaultVolume;
    strncpy(defaultAudioMode, audio["mode"] | "i2s", sizeof(defaultAudioMode));
  }
  
  // Parse I2S Defaults
  JsonObject i2s = doc["defaults"]["i2s"];
  if (!i2s.isNull()) {
    i2sDefaultPin = i2s["pin"] | i2sDefaultPin;
    i2sDefaultBuffer = i2s["bufferSize"] | i2sDefaultBuffer;
    i2sDefaultBuffers = i2s["numBuffers"] | i2sDefaultBuffers;
    i2sDefaultAmplitude = i2s["amplitude"] | i2sDefaultAmplitude;
  }
  
  // Parse PWM Defaults
  JsonObject pwm = doc["defaults"]["pwm"];
  if (!pwm.isNull()) {
    pwmDefaultPin = pwm["pin"] | pwmDefaultPin;
    pwmDefaultFreq = pwm["frequency"] | pwmDefaultFreq;
    pwmDefaultRes = pwm["resolution"] | pwmDefaultRes;
    pwmDefaultAmplitude = pwm["amplitude"] | pwmDefaultAmplitude;
    pwmDefaultGain = pwm["gain"] | pwmDefaultGain;
  }
  
  // Parse Effects Defaults
  JsonObject effects = doc["defaults"]["effects"];
  if (!effects.isNull()) {
    defaultEQEnabled = effects["eq"] | defaultEQEnabled;
    defaultFilterEnabled = effects["filter"] | defaultFilterEnabled;
    defaultReverbEnabled = effects["reverb"] | defaultReverbEnabled;
    defaultLFOEnabled = effects["lfo"] | defaultLFOEnabled;
    defaultDelayEnabled = effects["delay"] | defaultDelayEnabled;
  }
  
  // Parse Startup
  JsonObject startup = doc["startup"];
  if (!startup.isNull()) {
    strncpy(startupProfile, startup["profile"] | "default", sizeof(startupProfile));
  }
  
  // Parse Features
  JsonObject features = doc["features"];
  if (!features.isNull()) {
    enableAudioEngine = features["audio_engine"] | true;
    enableDisplay = features["display"] | false;
    enableWiFi = features["wifi"] | false;
    enableBluetooth = features["bluetooth"] | false;
  }
  
  Serial.println(F("[SYSCFG] ✓ Loaded system.json"));
  return true;
}

bool SystemConfig::saveToJSON(const char* path, AudioFilesystem* fs) {
  StaticJsonDocument<JSON_DOC_SIZE> doc;
  
  doc["version"] = SCHEMA_VERSION;
  
  // Startup
  doc["startup"]["profile"] = startupProfile;
  doc["startup"]["mode"] = "auto";
  
  // Features
  doc["features"]["audio_engine"] = enableAudioEngine;
  doc["features"]["display"] = enableDisplay;
  doc["features"]["wifi"] = enableWiFi;
  doc["features"]["bluetooth"] = enableBluetooth;
  
  // Defaults - Audio
  doc["defaults"]["audio"]["mode"] = defaultAudioMode;
  doc["defaults"]["audio"]["sampleRate"] = defaultSampleRate;
  doc["defaults"]["audio"]["voices"] = defaultMaxVoices;
  doc["defaults"]["audio"]["volume"] = defaultVolume;
  
  // Defaults - I2S
  doc["defaults"]["i2s"]["pin"] = i2sDefaultPin;
  doc["defaults"]["i2s"]["bufferSize"] = i2sDefaultBuffer;
  doc["defaults"]["i2s"]["numBuffers"] = i2sDefaultBuffers;
  doc["defaults"]["i2s"]["amplitude"] = i2sDefaultAmplitude;
  
  // Defaults - PWM
  doc["defaults"]["pwm"]["pin"] = pwmDefaultPin;
  doc["defaults"]["pwm"]["frequency"] = pwmDefaultFreq;
  doc["defaults"]["pwm"]["resolution"] = pwmDefaultRes;
  doc["defaults"]["pwm"]["amplitude"] = pwmDefaultAmplitude;
  doc["defaults"]["pwm"]["gain"] = pwmDefaultGain;
  
  // Defaults - Effects
  doc["defaults"]["effects"]["eq"] = defaultEQEnabled;
  doc["defaults"]["effects"]["filter"] = defaultFilterEnabled;
  doc["defaults"]["effects"]["reverb"] = defaultReverbEnabled;
  doc["defaults"]["effects"]["lfo"] = defaultLFOEnabled;
  doc["defaults"]["effects"]["delay"] = defaultDelayEnabled;
  
  File file = fs->open(path, "w");
  if (!file) {
    Serial.println(F("[SYSCFG] Cannot write system.json"));
    return false;
  }
  
  size_t written = serializeJsonPretty(doc, file);
  file.close();
  
  if (written > 0) {
    Serial.println(F("[SYSCFG] ✓ Saved system.json"));
    return true;
  }
  
  return false;
}