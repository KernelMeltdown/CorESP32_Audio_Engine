/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO FILESYSTEM - Implementation                                          ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioFilesystem.h"

AudioFilesystem::AudioFilesystem() : initialized(false) {}

bool AudioFilesystem::init() {
  Serial.println(F("[FS] Mounting SPIFFS..."));
  
  if (!SPIFFS.begin(FS_FORMAT_ON_FAIL)) {
    Serial.println(F("[ERROR] SPIFFS mount failed"));
    return false;
  }
  
  initialized = true;
  
  Serial.printf("[FS] ✓ Mounted: %d KB total, %d KB used\n", 
                totalBytes() / 1024, usedBytes() / 1024);
  
  ensureDirectories();
  
  return true;
}

void AudioFilesystem::deinit() {
  if (!initialized) return;
  SPIFFS.end();
  initialized = false;
}

void AudioFilesystem::ensureDirectories() {
  // Create directory structure if it doesn't exist
  mkdir(PATH_CONFIG);
  mkdir(PATH_PROFILES);
  mkdir(PATH_CODECS);
  mkdir(PATH_AUDIO);
  mkdir(PATH_MELODIES);
}

bool AudioFilesystem::exists(const char* path) {
  return SPIFFS.exists(path);
}

bool AudioFilesystem::remove(const char* path) {
  return SPIFFS.remove(path);
}

bool AudioFilesystem::rename(const char* oldPath, const char* newPath) {
  return SPIFFS.rename(oldPath, newPath);
}

File AudioFilesystem::open(const char* path, const char* mode) {
  return SPIFFS.open(path, mode);
}

bool AudioFilesystem::mkdir(const char* path) {
  return SPIFFS.mkdir(path);
}

bool AudioFilesystem::rmdir(const char* path) {
  return SPIFFS.rmdir(path);
}

void AudioFilesystem::listDir(const char* path) {
  File root = SPIFFS.open(path);
  if (!root || !root.isDirectory()) {
    Serial.println(F("[ERROR] Not a directory"));
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.printf("  [DIR]  %s\n", file.name());
    } else {
      Serial.printf("  [FILE] %s (%d bytes)\n", file.name(), file.size());
    }
    file = root.openNextFile();
  }
}

size_t AudioFilesystem::totalBytes() {
  return SPIFFS.totalBytes();
}

size_t AudioFilesystem::usedBytes() {
  return SPIFFS.usedBytes();
}

size_t AudioFilesystem::freeBytes() {
  return totalBytes() - usedBytes();
}
