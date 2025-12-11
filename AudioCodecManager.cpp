/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CODEC MANAGER - Implementation                                       ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#include "AudioCodecManager.h"
#include "AudioCodec_SAM.h"
#include "AudioFilesystem.h"

// ═══════════════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════

AudioCodecManager::AudioCodecManager() 
  : filesystem(nullptr)
  , wavCodec(nullptr)
  , samCodec(nullptr)
{
}

AudioCodecManager::~AudioCodecManager() {
  if (wavCodec) delete wavCodec;
  if (samCodec) delete samCodec;
}

// ═══════════════════════════════════════════════════════════════════════════
// Initialization
// ═══════════════════════════════════════════════════════════════════════════

void AudioCodecManager::init(AudioFilesystem* fs) {
  filesystem = fs;
  registerBuiltinCodecs();
}

void AudioCodecManager::registerBuiltinCodecs() {
  Serial.println("[CodecManager] Registering built-in codecs...");
  
  // WAV Codec
  wavCodec = new AudioCodec_WAV(filesystem);
  Serial.println("[CodecManager]   ✓ WAV Codec");
  
  // SAM Speech Synthesis Codec
  samCodec = new AudioCodec_SAM(filesystem);
  Serial.println("[CodecManager]   ✓ SAM Speech Synthesis");
}

// ═══════════════════════════════════════════════════════════════════════════
// Codec Detection
// ═══════════════════════════════════════════════════════════════════════════

AudioCodec* AudioCodecManager::detectCodec(const char* filename) {
  if (!filename) return nullptr;
  
  String fn(filename);
  fn.toLowerCase();
  
  // Check SAM first (text files or plain text)
  if (samCodec && (fn.endsWith(".txt") || fn.indexOf('.') == -1)) {
    if (samCodec->probe(filename)) {
      Serial.printf("[CodecManager] Detected: SAM for '%s'\n", filename);
      return samCodec;
    }
  }
  
  // Check WAV
  if (wavCodec && wavCodec->probe(filename)) {
    Serial.printf("[CodecManager] Detected: WAV for '%s'\n", filename);
    return wavCodec;
  }
  
  Serial.printf("[CodecManager] No codec found for '%s'\n", filename);
  return nullptr;
}

AudioCodec* AudioCodecManager::getCodec(const char* name) {
  if (!name) return nullptr;
  
  String n(name);
  n.toLowerCase();
  
  if (n == "wav" && wavCodec) return wavCodec;
  if (n == "sam" && samCodec) return samCodec;
  
  return nullptr;
}

bool AudioCodecManager::canDecode(const char* name, const char* filename) {
  AudioCodec* codec = getCodec(name);
  if (!codec) return false;
  return codec->probe(filename);
}

// ═══════════════════════════════════════════════════════════════════════════
// Information
// ═══════════════════════════════════════════════════════════════════════════

void AudioCodecManager::listCodecs() {
  Serial.println("\n╔═══════════════════════════════════════════════════════╗");
  Serial.println("║           REGISTERED AUDIO CODECS                    ║");
  Serial.println("╠═══════════════════════════════════════════════════════╣");
  
  if (wavCodec) printCodecLine(wavCodec);
  if (samCodec) printCodecLine(samCodec);
  
  Serial.println("╚═══════════════════════════════════════════════════════╝\n");
}

void AudioCodecManager::showCodecInfo(const char* name) {
  AudioCodec* codec = getCodec(name);
  if (!codec) {
    Serial.printf("Codec '%s' not found\n", name);
    return;
  }
  
  CodecCapabilities caps = codec->getCapabilities();
  const char** exts = codec->getExtensions();
  
  Serial.println("\n╔═══════════════════════════════════════════════════════╗");
  Serial.printf("║ Codec: %-44s║\n", codec->getName());
  Serial.printf("║ Version: %-42s║\n", codec->getVersion());
  Serial.println("╠═══════════════════════════════════════════════════════╣");
  Serial.printf("║ Decode:    %-42s║\n", caps.canDecode ? "✓" : "✗");
  Serial.printf("║ Encode:    %-42s║\n", caps.canEncode ? "✓" : "✗");
  Serial.printf("║ Seek:      %-42s║\n", caps.canSeek ? "✓" : "✗");
  Serial.printf("║ Streaming: %-42s║\n", caps.supportsStreaming ? "✓" : "✗");
  
  if (exts && exts[0]) {
    Serial.print("║ Extensions: ");
    for (int i = 0; exts[i]; i++) {
      Serial.print(exts[i]);
      if (exts[i+1]) Serial.print(", ");
    }
    Serial.println();
  }
  
  Serial.println("╚═══════════════════════════════════════════════════════╝\n");
}

void AudioCodecManager::printCodecLine(AudioCodec* codec) {
  if (!codec) return;
  
  const char** exts = codec->getExtensions();
  String extStr = "";
  if (exts && exts[0]) {
    for (int i = 0; exts[i] && i < 3; i++) {
      extStr += exts[i];
      if (exts[i+1] && i < 2) extStr += ",";
    }
  }
  
  Serial.printf("║ %-20s %-10s %-20s║\n", 
                codec->getName(),
                codec->getVersion(),
                extStr.c_str());
}

// ═══════════════════════════════════════════════════════════════════════════
// SAM-Specific Functions
// ═══════════════════════════════════════════════════════════════════════════

AudioCodec_SAM* AudioCodecManager::getSAMCodec() {
  return samCodec;
}

bool AudioCodecManager::speak(const String& text, SAMVoicePreset preset) {
  if (!samCodec) {
    Serial.println("[CodecManager] Error: SAM codec not available");
    return false;
  }
  
  samCodec->applyPreset(preset);
  return samCodec->synthesizeText(text);
}

bool AudioCodecManager::speakWithParams(const String& text, const SAMVoiceParams& params) {
  if (!samCodec) {
    Serial.println("[CodecManager] Error: SAM codec not available");
    return false;
  }
  
  samCodec->setVoiceParams(params);
  return samCodec->synthesizeText(text);
}