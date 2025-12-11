/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CODEC MANAGER - Header                                               ║
 ║  Manages all audio codecs including WAV and SAM Speech Synthesis            ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#ifndef AUDIO_CODEC_MANAGER_H
#define AUDIO_CODEC_MANAGER_H

#include <Arduino.h>
#include "AudioCodec.h"
#include "AudioCodec_WAV.h"
#include "AudioFilesystem.h"

// Forward declaration (SAM wird erst in .cpp inkludiert für schnellere Compile-Zeit)
class AudioCodec_SAM;

class AudioCodecManager {
public:
  AudioCodecManager();
  ~AudioCodecManager();
  
  // Initialize codec manager
  void init(AudioFilesystem* fs);
  
  // Codec detection
  AudioCodec* detectCodec(const char* filename);
  AudioCodec* getCodec(const char* name);
  bool canDecode(const char* name, const char* filename);
  
  // Information
  void listCodecs();
  void showCodecInfo(const char* name);
  
  // SAM-specific functions
  AudioCodec_SAM* getSAMCodec();
  bool speak(const String& text, SAMVoicePreset preset = SAMVoicePreset::NATURAL);
  bool speakWithParams(const String& text, const SAMVoiceParams& params);
  
private:
  AudioFilesystem* filesystem;
  
  // Registered codecs
  AudioCodec_WAV* wavCodec;
  AudioCodec_SAM* samCodec;
  
  // Helper functions
  void registerBuiltinCodecs();
  void printCodecLine(AudioCodec* codec);
};

#endif // AUDIO_CODEC_MANAGER_H