/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO PROFILE - Profile Management System                                  ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#ifndef AUDIO_PROFILE_H
#define AUDIO_PROFILE_H

#include <Arduino.h>
#include "AudioConfig.h"
#include "AudioSettings.h"
#include "AudioFilesystem.h"
#include <ArduinoJson.h>

class AudioProfile {
public:
  AudioProfile();
  
  void init(AudioFilesystem* fs);
  
  // Profile operations
  bool loadProfile(const char* name);
  bool saveProfile(const char* name);
  bool deleteProfile(const char* name);
  bool profileExists(const char* name);
  
  void listProfiles();
  void showProfileInfo(const char* name);
  
  // Default profile
  void createDefaultProfile();
  bool loadStartupProfile();
  bool setStartupProfile(const char* name);
  
  // Current settings
  AudioSettings* getCurrentSettings() { return &currentSettings; }
  
  // JSON import/export
  bool exportProfileJSON(const char* name);
  bool importProfileJSON();
  
  // Validation
  bool validateProfile(const char* name);
  
private:
  AudioFilesystem* filesystem;
  AudioSettings currentSettings;
  
  bool loadFromJSON(const char* path, AudioSettings& settings);
  bool saveToJSON(const char* path, const AudioSettings& settings);
  
  String getProfilePath(const char* name);
};

#endif // AUDIO_PROFILE_H
