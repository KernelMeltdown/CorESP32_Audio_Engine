// AudioCodecManager.cpp - Dynamic Codec Plugin Management

#include "AudioCodecManager.h"

AudioCodecManager::AudioCodecManager() 
  : filesystem(nullptr), codecCount(0) {
  for (int i = 0; i < MAX_CODEC_PLUGINS; i++) {
    codecs[i].codec = nullptr;
    codecs[i].builtin = false;
    codecs[i].active = false;
  }
}

AudioCodecManager::~AudioCodecManager() {
  for (int i = 0; i < codecCount; i++) {
    if (codecs[i].codec && codecs[i].active) {
      delete codecs[i].codec;
      codecs[i].codec = nullptr;
    }
  }
}

void AudioCodecManager::init(AudioFilesystem* fs) {
  filesystem = fs;
  codecCount = 0;
  
  Serial.println(F("[CODEC] Manager initialized"));
  
  registerBuiltinCodecs();
  
  if (filesystem && filesystem->isInitialized()) {
    scanForCodecPlugins();
  }
}

void AudioCodecManager::registerBuiltinCodecs() {
  Serial.println(F("[CODEC] Registering built-in codecs..."));
  
  AudioCodec_WAV* wavCodec = new AudioCodec_WAV(filesystem);
  if (registerCodec("wav", wavCodec, true)) {
    Serial.println(F("[CODEC]   ✓ WAV (PCM 8/16-bit, Mono/Stereo)"));
  }
}

bool AudioCodecManager::registerCodec(const char* name, AudioCodec* codec, bool builtin) {
  if (codecCount >= MAX_CODEC_PLUGINS) {
    Serial.println(F("[CODEC] ✗ Registry full!"));
    return false;
  }
  
  if (!codec) {
    return false;
  }
  
  strncpy(codecs[codecCount].name, name, sizeof(codecs[codecCount].name) - 1);
  codecs[codecCount].codec = codec;
  codecs[codecCount].builtin = builtin;
  codecs[codecCount].active = true;
  codecCount++;
  
  return true;
}

void AudioCodecManager::scanForCodecPlugins() {
  if (!filesystem->exists(PATH_CODECS)) {
    Serial.println(F("[CODEC] No /codecs directory (plugins disabled)"));
    return;
  }
  
  Serial.println(F("[CODEC] Scanning for plugins..."));
  
  File root = filesystem->open(PATH_CODECS, "r");
  if (!root || !root.isDirectory()) {
    return;
  }
  
  File file = root.openNextFile();
  bool foundAny = false;
  
  while (file) {
    if (!file.isDirectory()) {
      String filename = String(file.name());
      if (filename.endsWith(".so") || filename.endsWith(".bin")) {
        Serial.printf("[CODEC]   Found plugin: %s (not implemented yet)\n", file.name());
        foundAny = true;
      }
    }
    file = root.openNextFile();
  }
  
  if (!foundAny) {
    Serial.println(F("[CODEC]   (no plugins found)"));
  }
}

bool AudioCodecManager::loadCodecPlugin(const char* pluginPath) {
  Serial.printf("[CODEC] Plugin loading not yet implemented: %s\n", pluginPath);
  return false;
}

AudioCodec* AudioCodecManager::detectCodec(const char* filename) {
  String fn = String(filename);
  fn.toLowerCase();
  
  for (int i = 0; i < codecCount; i++) {
    if (!codecs[i].active) continue;
    
    const char** extensions = codecs[i].codec->getExtensions();
    
    for (int e = 0; extensions[e] != nullptr; e++) {
      if (fn.endsWith(extensions[e])) {
        if (codecs[i].codec->probe(filename)) {
          return codecs[i].codec;
        }
      }
    }
  }
  
  return nullptr;
}

AudioCodec* AudioCodecManager::getCodec(const char* name) {
  for (int i = 0; i < codecCount; i++) {
    if (!codecs[i].active) continue;
    
    if (strcasecmp(codecs[i].name, name) == 0) {
      return codecs[i].codec;
    }
  }
  
  return nullptr;
}

void AudioCodecManager::listCodecs() {
  Serial.println(F("\n╔════════════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                    AVAILABLE CODECS                            ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════════════╝\n"));
  
  if (codecCount == 0) {
    Serial.println(F("  No codecs registered!"));
    Serial.println();
    return;
  }
  
  Serial.println(F("  NAME    VERSION   TYPE        MEMORY    CPU     FORMATS"));
  Serial.println(F("  ────────────────────────────────────────────────────────────"));
  
  for (int i = 0; i < codecCount; i++) {
    if (!codecs[i].active) continue;
    
    CodecCapabilities caps = codecs[i].codec->getCapabilities();
    const char** exts = codecs[i].codec->getExtensions();
    
    String formats = "";
    for (int e = 0; exts[e] != nullptr; e++) {
      if (e > 0) formats += ", ";
      formats += exts[e];
    }
    
    Serial.printf("  %-7s %-9s %-11s %d KB     %.0f%%     %s\n",
                  codecs[i].codec->getName(),
                  codecs[i].codec->getVersion(),
                  codecs[i].builtin ? "Built-in" : "Plugin",
                  caps.ramUsage / 1024,
                  caps.cpuUsage * 100,
                  formats.c_str());
  }
  
  Serial.println();
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
  
  bool isBuiltin = false;
  for (int i = 0; i < codecCount; i++) {
    if (codecs[i].codec == codec) {
      isBuiltin = codecs[i].builtin;
      break;
    }
  }
  Serial.printf("Type:           %s\n", isBuiltin ? "Built-in" : "Plugin");
  
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
  Serial.print(F("  "));
  const char** exts = codec->getExtensions();
  for (int i = 0; exts[i] != nullptr; i++) {
    Serial.printf("%s ", exts[i]);
  }
  Serial.println();
  
  Serial.println(F("\nDependencies:   None"));
  Serial.printf("Removable:      %s\n", isBuiltin ? "No (built-in)" : "Yes");
  Serial.println();
}

bool AudioCodecManager::canDecode(const char* name, const char* filename) {
  AudioCodec* codec = getCodec(name);
  if (!codec) return false;
  
  return codec->probe(filename);
}