/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  ESP32 UNIVERSAL AUDIO OS v1.9.0                                             ║
 ║  Complete Audio Engine with LFO Modulation                                   ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioConfig.h"
#include "AudioEngine.h"
#include "AudioConsole.h"
#include "AudioProfile.h"
#include "AudioFilesystem.h"
#include "AudioCodecManager.h"

// ============================================================================
// GLOBAL INSTANCES
// ============================================================================

AudioEngine audio;
AudioProfile profile;
AudioFilesystem filesystem;
AudioCodecManager codecManager;
AudioConsole console;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println();
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║         ESP32 UNIVERSAL AUDIO OS v1.9.0                ║"));
  Serial.println(F("║         Initializing...                                ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
  Serial.println();
  
  // Initialize filesystem
  Serial.print(F("[INIT] Filesystem... "));
  if (filesystem.init()) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("FAILED (non-critical)"));
  }
  
  // Initialize profile manager
  Serial.print(F("[INIT] Profile Manager... "));
  profile.init(&filesystem);
  Serial.println(F("OK"));
  
  // Load startup profile (or create default)
  Serial.print(F("[INIT] Loading Profile... "));
  if (!profile.loadStartupProfile()) {
    Serial.println(F("Creating default..."));
    profile.createDefaultProfile();
  } else {
    Serial.println(F("OK"));
  }
  
  // ✅ Initialize codec manager (creates WAV codec internally)
  Serial.print(F("[INIT] Codec Manager... "));
  codecManager.init(&filesystem);
  Serial.println(F("OK"));
  
  // Initialize audio engine
  Serial.print(F("[INIT] Audio Engine... "));
  audio.init(profile.getCurrentSettings());
  Serial.println(F("OK"));
  
  // Initialize console
  Serial.print(F("[INIT] Console... "));
  console.init(&audio, &profile, &filesystem, &codecManager);
  Serial.println(F("OK"));
  
  Serial.println();
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║              INITIALIZATION COMPLETE                   ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
  Serial.println();
  
  // Show initial info
  Serial.printf("Profile:        %s\n", profile.getCurrentSettings()->name);
  Serial.printf("Audio Mode:     %s\n", audio.getModeName());
  Serial.printf("Sample Rate:    %u Hz\n", audio.getSampleRate());
  Serial.printf("Voices:         %d\n", audio.getVoiceCount());
  Serial.printf("Free RAM:       %d KB\n", ESP.getFreeHeap() / 1024);
  Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  audio.update();
  console.update();
}
