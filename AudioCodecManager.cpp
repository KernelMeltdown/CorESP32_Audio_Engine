/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CODEC MANAGER - Implementation                                       ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioCodecManager.h"

AudioCodecManager::AudioCodecManager() 
  : filesystem(nullptr), wavCodec(nullptr) {}

AudioCodecManager::~AudioCodecManager() {
  if (wavCodec) delete wavCodec;
}

void AudioCodecManager::init(AudioFilesystem* fs) {
  filesystem = fs;
  
  // Register built-in codecs
  wavCodec = new AudioCodec_WAV(filesystem);
  
  Serial.println(F("[CODEC] Manager initialized"));
  Serial.printf("[CODEC] Registered: %s\n", wavCodec->getName());
}

AudioCodec* AudioCodecManager::detectCodec(const char* filename) {
  String fn = String(filename);
  fn.toLowerCase();
  
  // Check WAV
  if (fn.endsWith(".wav") || fn.endsWith(".wave")) {
    if (wavCodec && wavCodec->probe(filename)) {
      return wavCodec;
    }
  }
  
  // Future: Check other codecs
  
  return nullptr;
}

void AudioCodecManager::listCodecs() {
  Serial.println(F("\n╔════════════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                    AVAILABLE CODECS                            ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════════════╝\n"));
  
  Serial.println(F("  NAME    VERSION   STATUS      MEMORY    CPU     FORMATS"));
  Serial.println(F("  ────────────────────────────────────────────────────────────"));
  
  // WAV
  if (wavCodec) {
    CodecCapabilities caps = wavCodec->getCapabilities();
    Serial.printf("  %-7s %-9s Built-in    %d KB     %.0f%%     .wav\n",
                  wavCodec->getName(),
                  wavCodec->getVersion(),
                  caps.ramUsage / 1024,
                  caps.cpuUsage * 100);
  }
  
  Serial.println();
}

AudioCodec* AudioCodecManager::getCodec(const char* name) {
  if (strcmp(name, "wav") == 0 || strcmp(name, "WAV") == 0) {
    return wavCodec;
  }
  
  return nullptr;
}

void AudioCodecManager::showCodecInfo(const char* name) {
  AudioCodec* codec = getCodec(name);
  if (!codec) {
    Serial.println(F("[ERROR] Codec not found"));
    return;
  }
  
  CodecCapabilities caps = codec->getCapabilities();
  
  Serial.println(F("\n╔════════════════════════════════════════════════════════════════╗"));
  Serial.printf("║  CODEC: %-54s ║\n", codec->getName());
  Serial.println(F("╚════════════════════════════════════════════════════════════════╝\n"));
  
  Serial.printf("Name:           %s\n", codec->getName());
  Serial.printf("Version:        %s\n", codec->getVersion());
  Serial.printf("Status:         Built-in\n");
  
  Serial.println(F("\nCapabilities:"));
  Serial.printf("  %s Decode\n", caps.canDecode ? "✓" : "✗");
  Serial.printf("  %s Encode\n", caps.canEncode ? "✓" : "✗");
  Serial.printf("  %s Real-time streaming\n", caps.canStream ? "✓" : "✗");
  Serial.printf("  %s Auto resampling\n", caps.canResample ? "✓" : "✗");
  
  Serial.println(F("\nSupported Formats:"));
  Serial.printf("  Sample Rates:  Up to %d Hz\n", caps.maxSampleRate);
  Serial.printf("  Channels:      Up to %d\n", caps.maxChannels);
  Serial.printf("  Bit Depths:    Up to %d-bit\n", caps.maxBitDepth);
  
  Serial.println(F("\nPerformance:"));
  Serial.printf("  Memory:        ~%d KB RAM\n", caps.ramUsage / 1024);
  Serial.printf("  CPU:           ~%.0f%% @ decode\n", caps.cpuUsage * 100);
  
  Serial.println(F("\nExtensions:"));
  const char** exts = codec->getExtensions();
  Serial.print(F("  "));
  for (int i = 0; exts[i] != nullptr; i++) {
    Serial.printf("%s ", exts[i]);
  }
  Serial.println();
  
  Serial.println(F("\nDependencies:   None"));
  Serial.println(F("Removable:      No (built-in)"));
  Serial.println();
}

bool AudioCodecManager::canDecode(const char* name, const char* filename) {
  AudioCodec* codec = getCodec(name);
  if (!codec) return false;
  
  return codec->probe(filename);
}
