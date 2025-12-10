/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CODEC - Base Interface for Codec Plugins                             ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#ifndef AUDIO_CODEC_H
#define AUDIO_CODEC_H

#include <Arduino.h>
#include "AudioConfig.h"

// ═══════════════════════════════════════════════════════════════════════════════
// CODEC CAPABILITIES
// ═══════════════════════════════════════════════════════════════════════════════

struct CodecCapabilities {
  bool canDecode;
  bool canEncode;
  bool canResample;
  bool canStream;
  
  uint32_t maxSampleRate;
  uint8_t maxChannels;
  uint8_t maxBitDepth;
  
  uint32_t ramUsage;      // Bytes
  float cpuUsage;         // 0.0-1.0
};

// ═══════════════════════════════════════════════════════════════════════════════
// AUDIO FORMAT INFO
// ═══════════════════════════════════════════════════════════════════════════════

struct AudioFormat {
  uint32_t sampleRate;
  uint8_t channels;
  uint8_t bitDepth;
  uint32_t bitrate;
  uint32_t duration;      // Seconds
  size_t dataSize;        // Bytes
};

// ═══════════════════════════════════════════════════════════════════════════════
// BASE CODEC CLASS (Interface)
// ═══════════════════════════════════════════════════════════════════════════════

class AudioCodec {
public:
  virtual ~AudioCodec() {}
  
  // Codec info
  virtual const char* getName() = 0;
  virtual const char* getVersion() = 0;
  virtual const char** getExtensions() = 0;
  virtual CodecCapabilities getCapabilities() = 0;
  
  // Probing
  virtual bool probe(const char* filename) = 0;
  
  // File operations
  virtual bool open(const char* filename) = 0;
  virtual void close() = 0;
  virtual bool isOpen() = 0;
  
  // Format info
  virtual AudioFormat getFormat() = 0;
  
  // Decoding
  virtual size_t read(int16_t* buffer, size_t samples) = 0;
  virtual bool seek(uint32_t sample) = 0;
  
  // Resampling control
  virtual void setTargetSampleRate(uint32_t rate) = 0;
  virtual uint32_t getTargetSampleRate() = 0;
};

#endif // AUDIO_CODEC_H
