// AudioFilesystem.cpp - SPIFFS Filesystem Manager

#include "AudioFilesystem.h"

AudioFilesystem::AudioFilesystem() : initialized(false) {}

bool AudioFilesystem::init() {
  Serial.println(F("[FS] Mounting SPIFFS..."));
  
  if (!SPIFFS.begin(FS_FORMAT_ON_FAIL)) {
    Serial.println(F("[FS] ✗ SPIFFS mount failed!"));
    Serial.println();
    Serial.println(F("=== SPIFFS SETUP REQUIRED ==="));
    Serial.println(F("1. Check partition scheme:"));
    Serial.println(F("   Arduino IDE: Tools → Partition Scheme → 'Default 4MB with spiffs'"));
    Serial.println(F("   PlatformIO: board_build.partitions = default.csv"));
    Serial.println();
    Serial.println(F("2. Upload SPIFFS data:"));
    Serial.println(F("   Arduino IDE: Tools → ESP32 Sketch Data Upload"));
    Serial.println(F("   PlatformIO: pio run --target uploadfs"));
    Serial.println();
    Serial.println(F("3. Create 'data/' folder structure:"));
    Serial.println(F("   data/"));
    Serial.println(F("   ├── config/"));
    Serial.println(F("   │   └── system.json"));
    Serial.println(F("   ├── profiles/"));
    Serial.println(F("   │   └── default.json"));
    Serial.println(F("   ├── melodies/"));
    Serial.println(F("   │   └── tetris.json"));
    Serial.println(F("   ├── audio/"));
    Serial.println(F("   └── codecs/"));
    Serial.println();
    Serial.println(F("Or set FS_FORMAT_ON_FAIL=true in AudioConfig.h (will erase data!)"));
    Serial.println(F("============================="));
    return false;
  }
  
  initialized = true;
  
  size_t total = totalBytes();
  size_t used = usedBytes();
  size_t free = total - used;
  
  Serial.printf("[FS] ✓ Mounted: %d KB total, %d KB used, %d KB free\n", 
                total / 1024, used / 1024, free / 1024);
  
  if (free < 10240) {
    Serial.println(F("[FS] ⚠ Warning: Less than 10KB free space!"));
  }
  
  ensureDirectories();
  verifyDataStructure();
  
  return true;
}

void AudioFilesystem::deinit() {
  if (!initialized) return;
  SPIFFS.end();
  initialized = false;
  Serial.println(F("[FS] Unmounted"));
}

void AudioFilesystem::ensureDirectories() {
  Serial.println(F("[FS] Ensuring directory structure..."));
  
  const char* dirs[] = {
    PATH_CONFIG,
    PATH_PROFILES,
    PATH_CODECS,
    PATH_AUDIO,
    PATH_MELODIES
  };
  
  for (const char* dir : dirs) {
    if (!SPIFFS.exists(dir)) {
      if (SPIFFS.mkdir(dir)) {
        Serial.printf("[FS]   ✓ Created: %s\n", dir);
      } else {
        Serial.printf("[FS]   ✗ Failed: %s\n", dir);
      }
    } else {
      Serial.printf("[FS]   ✓ Exists: %s\n", dir);
    }
  }
}

void AudioFilesystem::verifyDataStructure() {
  Serial.println(F("[FS] Verifying data structure..."));
  
  struct CheckFile {
    const char* path;
    const char* desc;
    bool required;
  };
  
  CheckFile files[] = {
    {PATH_SYSTEM_CONFIG, "System config", false},
    {"/profiles/default.json", "Default profile", true},
    {"/melodies/tetris.json", "Tetris melody", true}
  };
  
  bool allGood = true;
  
  for (const auto& f : files) {
    if (SPIFFS.exists(f.path)) {
      File file = SPIFFS.open(f.path, "r");
      Serial.printf("[FS]   ✓ %s (%d bytes)\n", f.desc, file.size());
      file.close();
    } else {
      if (f.required) {
        Serial.printf("[FS]   ✗ Missing: %s\n", f.desc);
        allGood = false;
      } else {
        Serial.printf("[FS]   ℹ Optional missing: %s\n", f.desc);
      }
    }
  }
  
  if (!allGood) {
    Serial.println();
    Serial.println(F("[FS] ⚠ Required files missing! Upload SPIFFS data folder."));
    Serial.println();
  }
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
  if (!root) {
    Serial.println(F("[ERROR] Cannot open directory"));
    return;
  }
  
  if (!root.isDirectory()) {
    Serial.println(F("[ERROR] Not a directory"));
    return;
  }
  
  File file = root.openNextFile();
  int count = 0;
  
  while (file) {
    if (file.isDirectory()) {
      Serial.printf("  [DIR]  %s\n", file.name());
    } else {
      Serial.printf("  [FILE] %-40s %8d bytes\n", file.name(), file.size());
      count++;
    }
    file = root.openNextFile();
  }
  
  if (count == 0) {
    Serial.println(F("  (empty)"));
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