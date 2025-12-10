/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  WAV CODEC - Implementation                                                 ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioCodec_WAV.h"

static const char* wavExtensions[] = {".wav", ".wave", nullptr};

AudioCodec_WAV::AudioCodec_WAV(AudioFilesystem* fs) 
  : filesystem(fs), fileOpen(false), targetSampleRate(0), 
    dataOffset(0), currentSample(0) {
  
  memset(&format, 0, sizeof(AudioFormat));
}

AudioCodec_WAV::~AudioCodec_WAV() {
  close();
}

const char* AudioCodec_WAV::getName() {
  return "WAV";
}

const char* AudioCodec_WAV::getVersion() {
  return "1.0.0";
}

const char** AudioCodec_WAV::getExtensions() {
  return wavExtensions;
}

CodecCapabilities AudioCodec_WAV::getCapabilities() {
  CodecCapabilities caps;
  caps.canDecode = true;
  caps.canEncode = false;
  caps.canResample = true;
  caps.canStream = true;
  caps.maxSampleRate = 48000;
  caps.maxChannels = 2;
  caps.maxBitDepth = 16;
  caps.ramUsage = 4096;
  caps.cpuUsage = 0.05f;
  return caps;
}

bool AudioCodec_WAV::probe(const char* filename) {
  if (!filesystem || !filesystem->isInitialized()) return false;
  
  File f = filesystem->open(filename, "r");
  if (!f) return false;
  
  char riff[4];
  f.read((uint8_t*)riff, 4);
  
  bool isWAV = (strncmp(riff, "RIFF", 4) == 0);
  f.close();
  
  return isWAV;
}

bool AudioCodec_WAV::open(const char* filename) {
  if (!filesystem || !filesystem->isInitialized()) return false;
  
  close();
  
  file = filesystem->open(filename, "r");
  if (!file) {
    Serial.println(F("[WAV] Failed to open file"));
    return false;
  }
  
  if (!parseWAVHeader()) {
    Serial.println(F("[WAV] Invalid WAV header"));
    file.close();
    return false;
  }
  
  fileOpen = true;
  currentSample = 0;
  
  Serial.println(F("[WAV] ✓ Opened"));
  Serial.printf("[WAV] Format: %d Hz, %d-bit, %s\n", 
                format.sampleRate, format.bitDepth, 
                format.channels == 2 ? "Stereo" : "Mono");
  
  return true;
}

void AudioCodec_WAV::close() {
  if (fileOpen) {
    file.close();
    fileOpen = false;
  }
}

bool AudioCodec_WAV::isOpen() {
  return fileOpen;
}

AudioFormat AudioCodec_WAV::getFormat() {
  return format;
}

bool AudioCodec_WAV::parseWAVHeader() {
  // Read RIFF header
  char riff[4];
  file.read((uint8_t*)riff, 4);
  if (strncmp(riff, "RIFF", 4) != 0) return false;
  
  file.read((uint8_t*)&format.dataSize, 4);  // File size - 8
  
  char wave[4];
  file.read((uint8_t*)wave, 4);
  if (strncmp(wave, "WAVE", 4) != 0) return false;
  
  // Find fmt chunk
  while (file.available()) {
    char chunkID[4];
    uint32_t chunkSize;
    
    file.read((uint8_t*)chunkID, 4);
    file.read((uint8_t*)&chunkSize, 4);
    
    if (strncmp(chunkID, "fmt ", 4) == 0) {
      uint16_t audioFormat;
      file.read((uint8_t*)&audioFormat, 2);
      
      if (audioFormat != 1) {  // PCM only
        Serial.println(F("[WAV] Only PCM supported"));
        return false;
      }
      
      uint16_t numChannels;
      file.read((uint8_t*)&numChannels, 2);
      format.channels = numChannels;
      
      file.read((uint8_t*)&format.sampleRate, 4);
      file.read((uint8_t*)&format.bitrate, 4);
      
      uint16_t blockAlign;
      file.read((uint8_t*)&blockAlign, 2);
      
      uint16_t bitsPerSample;
      file.read((uint8_t*)&bitsPerSample, 2);
      format.bitDepth = bitsPerSample;
      
      // Skip rest of chunk
      file.seek(file.position() + chunkSize - 16);
      
    } else if (strncmp(chunkID, "data", 4) == 0) {
      dataOffset = file.position();
      format.dataSize = chunkSize;
      format.duration = chunkSize / (format.sampleRate * format.channels * (format.bitDepth / 8));
      break;
      
    } else {
      // Skip unknown chunk
      file.seek(file.position() + chunkSize);
    }
  }
  
  return (dataOffset > 0);
}

size_t AudioCodec_WAV::read(int16_t* buffer, size_t samples) {
  if (!fileOpen) return 0;
  
  size_t samplesRead = 0;
  
  for (size_t i = 0; i < samples; i++) {
    if (!file.available()) break;
    
    int16_t sample = 0;
    
    if (format.bitDepth == 16) {
      file.read((uint8_t*)&sample, 2);
    } else if (format.bitDepth == 8) {
      uint8_t s8;
      file.read(&s8, 1);
      sample = (s8 - 128) << 8;  // 8-bit to 16-bit
    }
    
    // If stereo, mix to mono
    if (format.channels == 2) {
      int16_t right;
      if (format.bitDepth == 16) {
        file.read((uint8_t*)&right, 2);
      } else {
        uint8_t r8;
        file.read(&r8, 1);
        right = (r8 - 128) << 8;
      }
      sample = (sample + right) / 2;
    }
    
    buffer[i] = sample;
    samplesRead++;
    currentSample++;
  }
  
  return samplesRead;
}

bool AudioCodec_WAV::seek(uint32_t sample) {
  if (!fileOpen) return false;
  
  uint32_t bytePos = dataOffset + (sample * format.channels * (format.bitDepth / 8));
  
  if (file.seek(bytePos)) {
    currentSample = sample;
    return true;
  }
  
  return false;
}

void AudioCodec_WAV::setTargetSampleRate(uint32_t rate) {
  targetSampleRate = rate;
}

uint32_t AudioCodec_WAV::getTargetSampleRate() {
  return targetSampleRate;
}
