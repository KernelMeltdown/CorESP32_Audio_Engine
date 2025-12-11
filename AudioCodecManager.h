/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CODEC MANAGER - Codec Plugin Management System                       ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

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
  
  // Codec info
  void showCodecInfo(const char* name);
  bool canDecode(const char* name, const char* filename);
  
private:
  AudioFilesystem* filesystem;
  // Eigentlich sollten die codecs später im flieFS / spiffs liegen und beim booten automatisch erkannt werden oder so, bitte 
  // nochmal die DOCS studieren, wir wollen alles maximal flexibel haben und so was wie hier nicht hardcodieren
  AudioCodec_WAV* wavCodec;
  
  // Future: More codecs
  // AudioCodec_MP3* mp3Codec;
  // etc.
};

#endif // AUDIO_CODEC_MANAGER_H
