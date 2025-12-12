// ESP32 Audio OS v2.0 - Universal Audio Engine

#include "AudioConfig.h"
#include "SystemConfig.h"
#include "AudioFilesystem.h"
#include "AudioEngine.h"
#include "AudioProfile.h"
#include "AudioCodecManager.h"
#include "AudioConsole.h"

// ============================================================================
// GLOBAL INSTANCES
// ============================================================================
SystemConfig systemConfig;
AudioFilesystem filesystem;
AudioEngine audioEngine;
AudioProfile profileManager;
AudioCodecManager codecManager;
AudioConsole console;

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(500);
  
  printBanner();
  printSystemInfo();
  
  // 1. Initialize Filesystem
  if (!filesystem.init()) {
    Serial.println(F("\n[FATAL] Cannot proceed without filesystem!"));
    Serial.println(F("[FATAL] System halted. Please fix and reboot."));
    while(1) delay(1000);
  }
  
  // 2. Load System Configuration
  systemConfig.loadFromFile(&filesystem);
  
  // 3. Initialize Codec Manager
  codecManager.init(&filesystem);
  
  // 4. Initialize Profile Manager
  profileManager.init(&filesystem);
  
  // 5. Load Startup Profile
  if (!profileManager.loadStartupProfile()) {
    Serial.println(F("[WARN] No startup profile, creating default..."));
    profileManager.createDefaultProfile();
    profileManager.loadStartupProfile();
  }
  
  // 6. Initialize Audio Engine
  if (!audioEngine.init(profileManager.getCurrentSettings())) {
    Serial.println(F("\n[FATAL] Audio engine initialization failed!"));
    Serial.println(F("[FATAL] System halted. Please fix and reboot."));
    while(1) delay(1000);
  }
  
  // 7. Initialize Console
  console.init(&audioEngine, &profileManager, &filesystem, &codecManager);
  
  printReadyMessage();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  console.update();
  audioEngine.update();
  
  delay(1);
}

// ============================================================================
// BANNER & INFO
// ============================================================================
void printBanner() {
  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                                                                  ║"));
  Serial.println(F("║            ESP32 AUDIO OS v2.0                                   ║"));
  Serial.println(F("║                                                                  ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════════════╝"));
  Serial.println();
}

void printSystemInfo() {
  Serial.println(F("┌──────────────────────────────────────────────────────────────────┐"));
  Serial.println(F("│ SYSTEM INFORMATION                                               │"));
  Serial.println(F("├──────────────────────────────────────────────────────────────────┤"));
  Serial.printf ("│ Version:        %-48s │\n", AUDIO_OS_VERSION);
  Serial.printf ("│ Build Date:     %-48s │\n", AUDIO_OS_BUILD_DATE);
  Serial.printf ("│ Schema:         %-48s │\n", SCHEMA_VERSION);
  Serial.println(F("├──────────────────────────────────────────────────────────────────┤"));
  Serial.printf ("│ CPU:            %-48s │\n", ESP32_VARIANT);
  Serial.printf ("│ Frequency:      %-48s │\n", "160 MHz");
  Serial.printf ("│ Free RAM:       %-44d KB │\n", ESP.getFreeHeap() / 1024);
  
  #if HAS_DUAL_CORE
  Serial.printf ("│ Cores:          %-48s │\n", "Dual-Core (FreeRTOS)");
  #else
  Serial.printf ("│ Cores:          %-48s │\n", "Single-Core");
  #endif
  
  #if HAS_LP_CORE
  Serial.printf ("│ LP Core:        %-48s │\n", "Available");
  #endif
  
  Serial.println(F("├──────────────────────────────────────────────────────────────────┤"));
  Serial.println(F("│ FEATURES                                                         │"));
  Serial.println(F("├──────────────────────────────────────────────────────────────────┤"));
  Serial.println(F("│ ✓ I2S & PWM Audio Modes                                          │"));
  Serial.println(F("│ ✓ 5 Waveforms (Sine/Square/Saw/Triangle/Noise)                   │"));
  Serial.println(F("│ ✓ Polyphonic Synthesis (up to 8 voices)                          │"));
  Serial.println(F("│ ✓ ADSR Envelope Generator                                        │"));
  Serial.println(F("│ ✓ State-Variable Filter (LP/HP/BP)                               │"));
  Serial.println(F("│ ✓ Biquad 3-Band Parametric EQ                                    │"));
  Serial.println(F("│ ✓ Schroeder Reverb (Comb + Allpass)                              │"));
  Serial.println(F("│ ✓ LFO Modulation (Vibrato/Tremolo)                               │"));
  Serial.println(F("│ ✓ Delay/Echo Effect                                              │"));
  Serial.println(F("│ ✓ Smart Resampling (Linear/Cubic/Sinc)                           │"));
  Serial.println(F("│ ✓ Profile System (Load/Save/Export)                              │"));
  Serial.println(F("│ ✓ Dynamic Codec Plugin Architecture                              │"));
  Serial.println(F("│ ✓ SPIFFS Filesystem Integration                                  │"));
  Serial.println(F("│ ✓ Full Console Control                                           │"));
  
  #if USE_FIXED_POINT_MATH
  Serial.println(F("│ ✓ Fixed-Point Math Optimization                                  │"));
  #endif
  
  #if USE_WAVETABLE_LOOKUP
  Serial.println(F("│ ✓ Wavetable Synthesis                                            │"));
  #endif
  
  Serial.println(F("└──────────────────────────────────────────────────────────────────┘"));
  Serial.println();
}

void printReadyMessage() {
  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                       SYSTEM READY                               ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════════════╝"));
  Serial.println();
  Serial.println(F("Quick Start:"));
  Serial.println(F("  audio play tetris        - Play Tetris theme"));
  Serial.println(F("  audio waveform square    - 8-bit retro sound"));
  Serial.println(F("  audio filter on          - Enable filter"));
  Serial.println(F("  audio reverb on          - Enable reverb"));
  Serial.println(F("  audio help               - Show all commands"));
  Serial.println();
  Serial.println(F("Type 'audio help' for full command reference"));
  Serial.println();
  Serial.print(CONSOLE_PROMPT);
}