/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  WAV CODEC - WAV File Decoder                                               ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#ifndef AUDIO_CODEC_WAV_H
#define AUDIO_CODEC_WAV_H

#include "AudioCodec.h"
#include "AudioFilesystem.h"
#include <FS.h>

class AudioCodec_WAV : public AudioCodec {
public:
  AudioCodec_WAV(AudioFilesystem* fs);
  ~AudioCodec_WAV();
  
  // Codec info
  const char* getName() override;
  const char* getVersion() override;
  const char** getExtensions() override;
  CodecCapabilities getCapabilities() override;
  
  // Probing
  bool probe(const char* filename) override;
  
  // File operations
  bool open(const char* filename) override;
  void close() override;
  bool isOpen() override;
  
  // Format info
  AudioFormat getFormat() override;
  
  // Decoding
  size_t read(int16_t* buffer, size_t samples) override;
  bool seek(uint32_t sample) override;
  
  // Resampling
  void setTargetSampleRate(uint32_t rate) override;
  uint32_t getTargetSampleRate() override;
  
private:
  AudioFilesystem* filesystem;
  File file;
  bool fileOpen;
  
  AudioFormat format;
  uint32_t targetSampleRate;
  uint32_t dataOffset;
  uint32_t currentSample;
  
  bool parseWAVHeader();
  int16_t resampleRead();
};

#endif // AUDIO_CODEC_WAV_H
