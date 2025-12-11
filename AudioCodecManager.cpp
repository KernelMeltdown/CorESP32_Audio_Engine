/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CODEC MANAGER - Implementation                                       ║
 ║  WITH SAM SPEECH SYNTHESIS                                                   ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioCodecManager.h"
#include "AudioCodec_SAM.h"

AudioCodecManager::AudioCodecManager() 
  : filesystem(nullptr), wavCodec(nullptr), samCodec(nullptr) {}

AudioCodecManager::~AudioCodecManager() {
  if (wavCodec) delete wavCodec;
  if (samCodec) delete samCodec;
}

void AudioCodecManager::init(AudioFilesystem* fs) {
  filesystem = fs;
  
  // Register built-in codecs
  registerBuiltinCodecs();
  
  Serial.println(F("[CODEC] Manager initialized"));
}

void AudioCodecManager::registerBuiltinCodecs() {
  // WAV Codec
  wavCodec = new AudioCodec_WAV(filesystem);
  Serial.printf("[CODEC] Registered: %s v%s\n", 
                wavCodec->getName(), wavCodec->getVersion());
  
  // SAM Speech Synthesis
  samCodec = new AudioCodec_SAM(filesystem);
  Serial.printf("[CODEC] Registered: %s v%s\n", 
                samCodec->getName(), samCodec->getVersion());
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
  
  // Check SAM (Text ohne Extension oder .txt/.sam/.speech)
  if (samCodec && (fn.indexOf(".") == -1 || 
      fn.endsWith(".txt") || 
      fn.endsWith(".sam") || 
      fn.endsWith(".speech"))) {
    if (samCodec->probe(filename)) {
      return samCodec;
    }
  }
  
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
    printCodecLine(wavCodec);
  }
  
  // SAM
  if (samCodec) {
    printCodecLine(samCodec);
  }
  
  Serial.println();
}

void AudioCodecManager::printCodecLine(AudioCodec* codec) {
  if (!codec) return;
  
  CodecCapabilities caps = codec->getCapabilities();
  
  // Get extensions
  String exts = "";
  const char** extList = codec->getExtensions();
  for (int i = 0; extList[i] != nullptr && i < 3; i++) {
    if (i > 0) exts += " ";
    exts += ".";
    exts += extList[i];
  }
  
  Serial.printf("  %-7s %-9s Built-in    %2d KB     %3.0f%%     %s\n",
                codec->getName(),
                codec->getVersion(),
                caps.ramUsage / 1024,
                caps.cpuUsage * 100,
                exts.c_str());
}

AudioCodec* AudioCodecManager::getCodec(const char* name) {
  if (strcmp(name, "wav") == 0 || strcmp(name, "WAV") == 0) {
    return wavCodec;
  }
  
  if (strcmp(name, "sam") == 0 || strcmp(name, "SAM") == 0 || 
      strcmp(name, "speech") == 0) {
    return samCodec;
  }
  
  return nullptr;
}

AudioCodec_SAM* AudioCodecManager::getSAMCodec() {
  return samCodec;
}

bool AudioCodecManager::speak(const String& text, SAMVoicePreset preset) {
  if (!samCodec) {
    Serial.println(F("[CODEC] SAM not available!"));
    return false;
  }
  
  // Set voice preset
  samCodec->setVoicePreset(preset);
  
  // Synthesize text
  if (!samCodec->synthesizeText(text)) {
    Serial.println(F("[CODEC] Speech synthesis failed!"));
    return false;
  }
  
  Serial.printf("[CODEC] Synthesized: %u ms\n", samCodec->getDuration());
  return true;
}

bool AudioCodecManager::speakWithParams(const String& text, const SAMVoiceParams& params) {
  if (!samCodec) {
    Serial.println(F("[CODEC] SAM not available!"));
    return false;
  }
  
  // Set custom parameters
  samCodec->setVoiceParams(params);
  
  // Synthesize text
  if (!samCodec->synthesizeText(text)) {
    Serial.println(F("[CODEC] Speech synthesis failed!"));
    return false;
  }
  
  Serial.printf("[CODEC] Synthesized: %u ms\n", samCodec->getDuration());
  return true;
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
    Serial.printf(".%s ", exts[i]);
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