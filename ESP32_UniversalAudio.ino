/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        ESP32 UNIVERSAL AUDIO OS v1.9.0                       ║
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
  Serial.println(F("║           ESP32 UNIVERSAL AUDIO OS v1.9.0             ║"));
  Serial.println(F("║                   Initializing...                      ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
  Serial.println();

  // Initialize filesystem (CRITICAL!)
  Serial.print(F("[INIT] Filesystem... "));
  if (!filesystem.init()) {
    Serial.println(F("FAILED!"));
    Serial.println();
    Serial.println(F("╔════════════════════════════════════════════════════════╗"));
    Serial.println(F("║                    FILESYSTEM ERROR                    ║"));
    Serial.println(F("╚════════════════════════════════════════════════════════╝"));
    Serial.println();
    Serial.println(F("ERROR: SPIFFS mount failed!"));
    Serial.println(F(""));
    Serial.println(F("SOLUTION:"));
    Serial.println(F("  1. Tools -> Partition Scheme -> 'Default 4MB with spiffs'"));
    Serial.println(F("  2. Tools -> Erase Flash -> 'All Flash Contents'"));
    Serial.println(F("  3. Upload sketch again"));
    Serial.println();
    Serial.println(F("Or upload SPIFFS data folder:"));
    Serial.println(F("  1. Create 'data' folder in sketch directory"));
    Serial.println(F("  2. Add /melodies/tetris.json"));
    Serial.println(F("  3. Use 'ESP32 Sketch Data Upload' tool"));
    Serial.println();
    Serial.println(F("System halted. Please fix and reboot."));
    while(1) { delay(1000); }
  }
  Serial.println(F("OK"));

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

  // Initialize codec manager
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
  Serial.printf("Profile: %s\n", profile.getCurrentSettings()->name);
  Serial.printf("Audio Mode: %s\n", audio.getModeName());
  Serial.printf("Sample Rate: %u Hz\n", audio.getSampleRate());
  Serial.printf("Voices: %d\n", audio.getVoiceCount());
  Serial.printf("Free RAM: %d KB\n", ESP.getFreeHeap() / 1024);
  Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  audio.update();
  console.update();
}
