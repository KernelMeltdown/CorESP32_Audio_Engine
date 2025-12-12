// AudioFilesystem.h - SPIFFS Filesystem Manager

#ifndef AUDIO_FILESYSTEM_H
#define AUDIO_FILESYSTEM_H

#include <Arduino.h>
#include "AudioConfig.h"
#include <FS.h>
#include <SPIFFS.h>

class AudioFilesystem {
public:
  AudioFilesystem();
  
  bool init();
  void deinit();
  
  // File operations
  bool exists(const char* path);
  bool remove(const char* path);
  bool rename(const char* oldPath, const char* newPath);
  File open(const char* path, const char* mode);
  
  // Directory operations
  bool mkdir(const char* path);
  bool rmdir(const char* path);
  void listDir(const char* path);
  
  // Info
  size_t totalBytes();
  size_t usedBytes();
  size_t freeBytes();
  
  bool isInitialized() { return initialized; }
  
private:
  bool initialized;
  
  void ensureDirectories();
  void verifyDataStructure();
};

#endif // AUDIO_FILESYSTEM_H