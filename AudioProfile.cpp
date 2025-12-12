// AudioProfile.cpp - Profile Management System

#include "AudioProfile.h"

AudioProfile::AudioProfile()
  : filesystem(nullptr) {}

void AudioProfile::init(AudioFilesystem* fs) {
  filesystem = fs;
}

String AudioProfile::getProfilePath(const char* name) {
  String path = PATH_PROFILES;
  path += "/";
  path += name;
  path += ".json";
  return path;
}

bool AudioProfile::loadProfile(const char* name) {
  if (!filesystem || !filesystem->isInitialized()) {
    Serial.println(F("[ERROR] Filesystem not initialized"));
    return false;
  }

  String path = getProfilePath(name);

  Serial.printf("[PROFILE] Loading '%s'...\n", name);

  if (!filesystem->exists(path.c_str())) {
    Serial.println(F("[ERROR] Profile not found"));
    return false;
  }

  if (loadFromJSON(path.c_str(), currentSettings)) {
    Serial.println(F("[PROFILE] ✓ Loaded successfully"));
    return true;
  }

  Serial.println(F("[ERROR] Failed to parse profile"));
  return false;
}

bool AudioProfile::saveProfile(const char* name) {
  if (!filesystem || !filesystem->isInitialized()) {
    Serial.println(F("[ERROR] Filesystem not initialized"));
    return false;
  }

  String path = getProfilePath(name);

  Serial.printf("[PROFILE] Saving as '%s'...\n", name);

  strncpy(currentSettings.name, name, sizeof(currentSettings.name) - 1);

  if (saveToJSON(path.c_str(), currentSettings)) {
    Serial.println(F("[PROFILE] ✓ Saved successfully"));
    return true;
  }

  Serial.println(F("[ERROR] Failed to save profile"));
  return false;
}

bool AudioProfile::deleteProfile(const char* name) {
  if (!filesystem || !filesystem->isInitialized()) return false;

  if (strcmp(name, "default") == 0) {
    Serial.println(F("[ERROR] Cannot delete default profile"));
    return false;
  }

  String path = getProfilePath(name);

  if (filesystem->remove(path.c_str())) {
    Serial.printf("[PROFILE] Deleted '%s'\n", name);
    return true;
  }

  return false;
}

bool AudioProfile::profileExists(const char* name) {
  if (!filesystem || !filesystem->isInitialized()) return false;
  String path = getProfilePath(name);
  return filesystem->exists(path.c_str());
}

void AudioProfile::listProfiles() {
  if (!filesystem || !filesystem->isInitialized()) {
    Serial.println(F("[ERROR] Filesystem not initialized"));
    return;
  }

  Serial.println(F("\n╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                 AVAILABLE PROFILES                     ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝\n"));

  File root = filesystem->open(PATH_PROFILES, "r");
  if (!root || !root.isDirectory()) {
    Serial.println(F("  No profiles found"));
    return;
  }

  File file = root.openNextFile();
  bool found = false;

  while (file) {
    if (!file.isDirectory()) {
      String filename = String(file.name());
      if (filename.endsWith(".json")) {
        found = true;
        
        int lastSlash = filename.lastIndexOf('/');
        if (lastSlash >= 0) {
          filename = filename.substring(lastSlash + 1);
        }
        filename.replace(".json", "");

        bool isCurrent = (filename == String(currentSettings.name));

        Serial.printf("  %s %s\n",
                      filename.c_str(),
                      isCurrent ? "[*]" : "");
      }
    }
    file = root.openNextFile();
  }

  if (!found) {
    Serial.println(F("  No profiles found"));
  } else {
    Serial.println(F("\n  [*] = currently active"));
  }
  Serial.println();
}

void AudioProfile::showProfileInfo(const char* name) {
  if (!filesystem || !filesystem->isInitialized()) return;

  String path = getProfilePath(name);

  if (!filesystem->exists(path.c_str())) {
    Serial.println(F("[ERROR] Profile not found"));
    return;
  }

  AudioSettings temp;
  if (!loadFromJSON(path.c_str(), temp)) {
    Serial.println(F("[ERROR] Failed to load profile"));
    return;
  }

  Serial.println(F("\n╔════════════════════════════════════════════════════════╗"));
  Serial.printf("║  PROFILE: %-44s ║\n", temp.name);
  Serial.println(F("╚════════════════════════════════════════════════════════╝\n"));

  Serial.printf("Description:   %s\n", temp.description);
  Serial.printf("Audio Mode:    %s\n", temp.getModeName());
  Serial.printf("Sample Rate:   %d Hz\n", temp.sampleRate);
  Serial.printf("Voices:        %d\n", temp.voices);
  Serial.printf("Volume:        %d/255\n", temp.volume);

  if (temp.mode == AUDIO_MODE_I2S) {
    Serial.printf("\nI2S Settings:\n");
    Serial.printf("  Pin:         GPIO %d\n", temp.i2s.pin);
    Serial.printf("  Buffer:      %d samples\n", temp.i2s.bufferSize);
    Serial.printf("  Buffers:     %d\n", temp.i2s.numBuffers);
    Serial.printf("  Amplitude:   %d\n", temp.i2s.amplitude);
  } else {
    Serial.printf("\nPWM Settings:\n");
    Serial.printf("  Pin:         GPIO %d\n", temp.pwm.pin);
    Serial.printf("  Frequency:   %d Hz\n", temp.pwm.frequency);
    Serial.printf("  Resolution:  %d bits\n", temp.pwm.resolution);
    Serial.printf("  Amplitude:   %d\n", temp.pwm.amplitude);
    Serial.printf("  Gain:        %d×\n", temp.pwm.gain);
  }

  Serial.printf("\nEffects:\n");
  Serial.printf("  EQ:          %s (Bass %+d, Mid %+d, Treble %+d dB)\n",
                temp.eq.enabled ? "On" : "Off",
                temp.eq.bass, temp.eq.mid, temp.eq.treble);
                
  Serial.printf("  Filter:      %s (%s, %.0fHz, Q:%.2f)\n",
                temp.filter.enabled ? "On" : "Off",
                temp.filter.getTypeName(),
                temp.filter.cutoff,
                temp.filter.resonance);
                
  Serial.printf("  Reverb:      %s (Room:%.2f, Damp:%.2f, Wet:%.2f)\n",
                temp.reverb.enabled ? "On" : "Off",
                temp.reverb.roomSize,
                temp.reverb.damping,
                temp.reverb.wet);
                
  Serial.printf("  LFO:         %s (Rate:%.1fHz, Depth:%.0f%%)\n",
                temp.lfo.enabled ? "On" : "Off",
                temp.lfo.rate,
                temp.lfo.depth);
                
  Serial.printf("  Delay:       %s (%dms, FB:%d%%, Mix:%d%%)\n",
                temp.delay.enabled ? "On" : "Off",
                temp.delay.timeMs,
                temp.delay.feedback,
                temp.delay.mix);

  Serial.printf("\nResampling:    %s\n", temp.getResampleQualityName());
  Serial.println();
}

void AudioProfile::createDefaultProfile() {
  Serial.println(F("[PROFILE] Creating default profile..."));

  currentSettings = AudioSettings();

  if (filesystem && filesystem->isInitialized()) {
    saveProfile("default");
  }
}

bool AudioProfile::loadStartupProfile() {
  if (profileExists("default")) {
    return loadProfile("default");
  }

  return false;
}

bool AudioProfile::setStartupProfile(const char* name) {
  if (!profileExists(name)) {
    Serial.println(F("[ERROR] Profile not found"));
    return false;
  }

  Serial.printf("[CONFIG] Setting '%s' as startup profile\n", name);
  return true;
}

bool AudioProfile::loadFromJSON(const char* path, AudioSettings& settings) {
  File file = filesystem->open(path, "r");
  if (!file) return false;

  StaticJsonDocument<JSON_DOC_SIZE> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.printf("[ERROR] JSON parse: %s\n", error.c_str());
    return false;
  }

  strncpy(settings.name, doc["name"] | "default", sizeof(settings.name) - 1);
  strncpy(settings.description, doc["description"] | "", sizeof(settings.description) - 1);

  const char* mode = doc["audio"]["mode"] | "i2s";
  settings.setMode(mode);

  settings.sampleRate = doc["audio"]["sampleRate"] | 22050;
  settings.voices = doc["audio"]["voices"] | 4;
  settings.volume = doc["audio"]["volume"] | 200;

  settings.i2s.pin = doc["hardware"]["i2s"]["pin"] | 1;
  settings.i2s.bufferSize = doc["hardware"]["i2s"]["bufferSize"] | 128;
  settings.i2s.numBuffers = doc["hardware"]["i2s"]["numBuffers"] | 4;
  settings.i2s.amplitude = doc["hardware"]["i2s"]["amplitude"] | 12000;

  settings.pwm.pin = doc["hardware"]["pwm"]["pin"] | 2;
  settings.pwm.frequency = doc["hardware"]["pwm"]["frequency"] | 78125;
  settings.pwm.resolution = doc["hardware"]["pwm"]["resolution"] | 9;
  settings.pwm.amplitude = doc["hardware"]["pwm"]["amplitude"] | 5000;
  settings.pwm.gain = doc["hardware"]["pwm"]["gain"] | 7;

  JsonObject eqObj = doc["effects"]["eq"];
  settings.eq.enabled = eqObj["enabled"] | false;
  settings.eq.bass = eqObj["bass"] | 0;
  settings.eq.mid = eqObj["mid"] | 0;
  settings.eq.treble = eqObj["treble"] | 0;

  JsonObject filterObj = doc["effects"]["filter"];
  settings.filter.enabled = filterObj["enabled"] | false;
  settings.filter.setType(filterObj["type"] | "lowpass");
  settings.filter.cutoff = filterObj["cutoff"] | 1000.0f;
  settings.filter.resonance = filterObj["resonance"] | 0.1f;

  JsonObject reverbObj = doc["effects"]["reverb"];
  settings.reverb.enabled = reverbObj["enabled"] | false;
  settings.reverb.roomSize = reverbObj["roomSize"] | 0.5f;
  settings.reverb.damping = reverbObj["damping"] | 0.5f;
  settings.reverb.wet = reverbObj["wet"] | 0.33f;

  JsonObject lfoObj = doc["effects"]["lfo"];
  settings.lfo.enabled = lfoObj["enabled"] | false;
  settings.lfo.vibratoEnabled = lfoObj["vibratoEnabled"] | false;
  settings.lfo.tremoloEnabled = lfoObj["tremoloEnabled"] | false;
  settings.lfo.rate = lfoObj["rate"] | 5.0f;
  settings.lfo.depth = lfoObj["depth"] | 20.0f;

  JsonObject delayObj = doc["effects"]["delay"];
  settings.delay.enabled = delayObj["enabled"] | false;
  settings.delay.timeMs = delayObj["timeMs"] | 250;
  settings.delay.feedback = delayObj["feedback"] | 50;
  settings.delay.mix = delayObj["mix"] | 30;

  const char* resample = doc["resample"]["quality"] | "best";
  settings.setResampleQuality(resample);
  
  return true;
}

bool AudioProfile::saveToJSON(const char* path, const AudioSettings& settings) {
  StaticJsonDocument<JSON_DOC_SIZE> doc;

  doc["schema_version"] = SCHEMA_VERSION;
  doc["engine_version"] = AUDIO_OS_VERSION;
  doc["name"] = settings.name;
  doc["description"] = settings.description;

  doc["audio"]["mode"] = settings.getModeName();
  doc["audio"]["sampleRate"] = settings.sampleRate;
  doc["audio"]["voices"] = settings.voices;
  doc["audio"]["volume"] = settings.volume;

  doc["hardware"]["i2s"]["pin"] = settings.i2s.pin;
  doc["hardware"]["i2s"]["bufferSize"] = settings.i2s.bufferSize;
  doc["hardware"]["i2s"]["numBuffers"] = settings.i2s.numBuffers;
  doc["hardware"]["i2s"]["amplitude"] = settings.i2s.amplitude;

  doc["hardware"]["pwm"]["pin"] = settings.pwm.pin;
  doc["hardware"]["pwm"]["frequency"] = settings.pwm.frequency;
  doc["hardware"]["pwm"]["resolution"] = settings.pwm.resolution;
  doc["hardware"]["pwm"]["amplitude"] = settings.pwm.amplitude;
  doc["hardware"]["pwm"]["gain"] = settings.pwm.gain;

  JsonObject eqObj = doc["effects"].createNestedObject("eq");
  eqObj["enabled"] = settings.eq.enabled;
  eqObj["bass"] = settings.eq.bass;
  eqObj["mid"] = settings.eq.mid;
  eqObj["treble"] = settings.eq.treble;

  JsonObject filterObj = doc["effects"].createNestedObject("filter");
  filterObj["enabled"] = settings.filter.enabled;
  filterObj["type"] = settings.filter.getTypeName();
  filterObj["cutoff"] = settings.filter.cutoff;
  filterObj["resonance"] = settings.filter.resonance;

  JsonObject reverbObj = doc["effects"].createNestedObject("reverb");
  reverbObj["enabled"] = settings.reverb.enabled;
  reverbObj["roomSize"] = settings.reverb.roomSize;
  reverbObj["damping"] = settings.reverb.damping;
  reverbObj["wet"] = settings.reverb.wet;

  JsonObject lfoObj = doc["effects"].createNestedObject("lfo");
  lfoObj["enabled"] = settings.lfo.enabled;
  lfoObj["vibratoEnabled"] = settings.lfo.vibratoEnabled;
  lfoObj["tremoloEnabled"] = settings.lfo.tremoloEnabled;
  lfoObj["rate"] = settings.lfo.rate;
  lfoObj["depth"] = settings.lfo.depth;

  JsonObject delayObj = doc["effects"].createNestedObject("delay");
  delayObj["enabled"] = settings.delay.enabled;
  delayObj["timeMs"] = settings.delay.timeMs;
  delayObj["feedback"] = settings.delay.feedback;
  delayObj["mix"] = settings.delay.mix;

  doc["resample"]["quality"] = settings.getResampleQualityName();

  File file = filesystem->open(path, "w");
  if (!file) return false;

  size_t written = serializeJsonPretty(doc, file);
  file.close();

  return (written > 0);
}

bool AudioProfile::validateProfile(const char* name) {
  AudioSettings temp;
  String path = getProfilePath(name);

  if (!filesystem->exists(path.c_str())) {
    Serial.println(F("[ERROR] Profile not found"));
    return false;
  }

  if (!loadFromJSON(path.c_str(), temp)) {
    Serial.println(F("[ERROR] Invalid JSON"));
    return false;
  }

  Serial.println(F("[VALIDATE] ✓ Profile is valid"));
  return true;
}

bool AudioProfile::exportProfileJSON(const char* name) {
  String path = getProfilePath(name);

  if (!filesystem->exists(path.c_str())) {
    Serial.println(F("[ERROR] Profile not found"));
    return false;
  }

  File file = filesystem->open(path.c_str(), "r");
  if (!file) return false;

  Serial.println(F("\n--- BEGIN PROFILE JSON ---"));
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println(F("\n--- END PROFILE JSON ---\n"));

  file.close();
  return true;
}

bool AudioProfile::importProfileJSON() {
  Serial.println(F("[IMPORT] Paste JSON, end with '###' on new line:"));

  String jsonData = "";
  unsigned long timeout = millis() + 30000;

  while (millis() < timeout) {
    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();

      if (line == "###") break;

      jsonData += line + "\n";
    }
    delay(10);
  }

  if (jsonData.length() == 0) {
    Serial.println(F("[ERROR] No data received"));
    return false;
  }

  StaticJsonDocument<JSON_DOC_SIZE> doc;
  DeserializationError error = deserializeJson(doc, jsonData);

  if (error) {
    Serial.printf("[ERROR] Invalid JSON: %s\n", error.c_str());
    return false;
  }

  const char* name = doc["name"] | "imported";
  String path = getProfilePath(name);

  File file = filesystem->open(path.c_str(), "w");
  if (!file) {
    Serial.println(F("[ERROR] Failed to create file"));
    return false;
  }

  file.print(jsonData);
  file.close();

  Serial.printf("[IMPORT] ✓ Profile '%s' imported\n", name);
  return true;
}