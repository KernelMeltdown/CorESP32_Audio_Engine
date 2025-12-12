// AudioCodecManager.h - Dynamic Codec Plugin Management

#ifndef AUDIO_CODEC_MANAGER_H
#define AUDIO_CODEC_MANAGER_H

#include <Arduino.h>
#include "AudioConfig.h"
#include "AudioCodec.h"
#include "AudioCodec_WAV.h"
#include "AudioFilesystem.h"

class AudioCodecManager {
public:
  AudioCodecManager();
  ~AudioCodecManager();
  
  void init(AudioFilesystem* fs);
  
  // Codec detection
  AudioCodec* detectCodec(const char* filename);
  
  // Codec registry
  void listCodecs();
  AudioCodec* getCodec(const char* name);
  int getCodecCount() { return codecCount; }
  
  // Codec info
  void showCodecInfo(const char* name);
  bool canDecode(const char* name, const char* filename);
  
  // Plugin loading (future)
  bool loadCodecPlugin(const char* pluginPath);
  void scanForCodecPlugins();
  
private:
  AudioFilesystem* filesystem;
  
  struct CodecEntry {
    char name[32];
    AudioCodec* codec;
    bool builtin;
    bool active;
  };
  
  CodecEntry codecs[MAX_CODEC_PLUGINS];
  int codecCount;
  
  void registerBuiltinCodecs();
  bool registerCodec(const char* name, AudioCodec* codec, bool builtin);
};

#endif // AUDIO_CODEC_MANAGER_H