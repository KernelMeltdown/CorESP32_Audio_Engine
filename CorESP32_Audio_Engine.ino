/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  ESP32 UNIVERSAL AUDIO ENGINE                                               ║
 ║  Main Application with SAM Speech Synthesis                                 ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioEngine.h"
#include "AudioCodecManager.h"
#include "AudioCodec_SAM.h"
#include "AudioFilesystem.h"
#include "AudioConsole.h"

// ═══════════════════════════════════════════════════════════════════════════
// Global Instances
// ═══════════════════════════════════════════════════════════════════════════

AudioEngine audioEngine;
AudioCodecManager codecManager;
AudioFilesystem filesystem;
AudioConsole console;

// ═══════════════════════════════════════════════════════════════════════════
// Setup
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔═══════════════════════════════════════════════════════╗");
    Serial.println("║  ESP32 UNIVERSAL AUDIO ENGINE - SAM Integration     ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝\n");
    
    // Initialize filesystem
    Serial.println("[Setup] Initializing filesystem...");
    if (!filesystem.begin()) {
        Serial.println("[Setup] ERROR: Filesystem init failed");
    }
    
    // Initialize audio engine
    Serial.println("[Setup] Initializing audio engine...");
    audioEngine.init();
    
    // Initialize codec manager
    Serial.println("[Setup] Initializing codec manager...");
    codecManager.init(&filesystem);
    
    // List available codecs
    codecManager.listCodecs();
    
    // Initialize console
    console.begin(&audioEngine, &codecManager, &filesystem);
    
    Serial.println("\n[Setup] System ready!");
    Serial.println("Commands:");
    Serial.println("  speak <text>          - Speak text");
    Serial.println("  voice <preset>        - Change voice (natural/clear/warm/robot/child/deep)");
    Serial.println("  test                  - Run SAM tests");
    Serial.println("");
}

// ═══════════════════════════════════════════════════════════════════════════
// Loop
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
    console.update();
    delay(10);
}

// ═══════════════════════════════════════════════════════════════════════════
// SAM Test Functions
// ═══════════════════════════════════════════════════════════════════════════

void testSAMBasic() {
    Serial.println("\n═══ TEST 1: Basic SAM Synthesis ═══");
    
    AudioCodec_SAM* sam = codecManager.getSAMCodec();
    if (!sam) {
        Serial.println("ERROR: SAM codec not available");
        return;
    }
    
    sam->applyPreset(SAMVoicePreset::NATURAL);
    sam->open("Hello ESP32. This is SAM speaking.");
    
    if (sam->isOpen()) {
        AudioFormat fmt = sam->getFormat();
        Serial.printf("Format: %u Hz, %u-bit, %u ch\n", 
                      fmt.sampleRate, fmt.bitsPerSample, fmt.channels);
        Serial.printf("Duration: %u ms\n", fmt.duration);
        
        // Read and play audio
        int16_t buffer[512];
        size_t totalRead = 0;
        while (true) {
            size_t read = sam->read(buffer, sizeof(buffer));
            if (read == 0) break;
            totalRead += read;
            // TODO: Send to audio output here
        }
        
        Serial.printf("Total bytes read: %u\n", totalRead);
        sam->close();
    } else {
        Serial.println("ERROR: Failed to synthesize");
    }
}

void testSAMVoices() {
    Serial.println("\n═══ TEST 2: Voice Presets ═══");
    
    AudioCodec_SAM* sam = codecManager.getSAMCodec();
    if (!sam) return;
    
    const char* text = "Testing voice preset.";
    
    struct {
        SAMVoicePreset preset;
        const char* name;
    } presets[] = {
        {SAMVoicePreset::NATURAL, "Natural"},
        {SAMVoicePreset::CLEAR, "Clear"},
        {SAMVoicePreset::WARM, "Warm"},
        {SAMVoicePreset::ROBOT, "Robot"},
        {SAMVoicePreset::CHILD, "Child"},
        {SAMVoicePreset::DEEP, "Deep"}
    };
    
    for (int i = 0; i < 6; i++) {
        Serial.printf("\nTesting %s voice...\n", presets[i].name);
        sam->applyPreset(presets[i].preset);
        sam->synthesizeText(text);
        delay(100);
    }
}

void testSAMCustomParams() {
    Serial.println("\n═══ TEST 3: Custom Voice Parameters ═══");
    
    AudioCodec_SAM* sam = codecManager.getSAMCodec();
    if (!sam) return;
    
    SAMVoiceParams params;
    params.speed = 80;
    params.pitch = 75;
    params.throat = 110;
    params.mouth = 128;
    params.stress = 0;
    
    sam->setVoiceParams(params);
    sam->synthesizeText("Custom voice parameters test.");
    
    Serial.println("Custom parameters applied");
}

void testSAMViaManager() {
    Serial.println("\n═══ TEST 4: Codec Manager Integration ═══");
    
    // Use high-level API
    codecManager.speak("Testing codec manager.", SAMVoicePreset::NATURAL);
    
    // Test codec detection
    AudioCodec* codec = codecManager.detectCodec("Hello World");
    if (codec) {
        Serial.printf("Detected codec: %s\n", codec->getName());
        codec->open("Hello World");
        codec->close();
    }
}

void testSAMTextFile() {
    Serial.println("\n═══ TEST 5: Text File Input ═══");
    
    AudioCodec_SAM* sam = codecManager.getSAMCodec();
    if (!sam) return;
    
    // Try to load from file (if it exists)
    sam->open("/test_speech.txt");
    
    // If file doesn't exist, it will use the filename as text
    Serial.println("Test complete");
}

void runAllSAMTests() {
    Serial.println("\n╔═══════════════════════════════════════════════════════╗");
    Serial.println("║          RUNNING SAM TEST SUITE                      ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    
    testSAMBasic();
    delay(500);
    
    testSAMVoices();
    delay(500);
    
    testSAMCustomParams();
    delay(500);
    
    testSAMViaManager();
    delay(500);
    
    testSAMTextFile();
    
    Serial.println("\n╔═══════════════════════════════════════════════════════╗");
    Serial.println("║          ALL TESTS COMPLETE                          ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝\n");
}

// ═══════════════════════════════════════════════════════════════════════════
// Console Command Handler (called from AudioConsole)
// ═══════════════════════════════════════════════════════════════════════════

void handleSAMCommand(const String& cmd, const String& args) {
    if (cmd == "speak" && !args.isEmpty()) {
        Serial.printf("Speaking: '%s'\n", args.c_str());
        codecManager.speak(args, SAMVoicePreset::NATURAL);
    }
    else if (cmd == "voice" && !args.isEmpty()) {
        SAMVoicePreset preset = SAMVoicePreset::NATURAL;
        String v = args;
        v.toLowerCase();
        
        if (v == "natural") preset = SAMVoicePreset::NATURAL;
        else if (v == "clear") preset = SAMVoicePreset::CLEAR;
        else if (v == "warm") preset = SAMVoicePreset::WARM;
        else if (v == "robot") preset = SAMVoicePreset::ROBOT;
        else if (v == "child") preset = SAMVoicePreset::CHILD;
        else if (v == "deep") preset = SAMVoicePreset::DEEP;
        else {
            Serial.println("Unknown voice preset");
            return;
        }
        
        AudioCodec_SAM* sam = codecManager.getSAMCodec();
        if (sam) {
            sam->applyPreset(preset);
            Serial.printf("Voice set to: %s\n", args.c_str());
        }
    }
    else if (cmd == "test") {
        runAllSAMTests();
    }
    else {
        Serial.println("Unknown SAM command");
    }
}