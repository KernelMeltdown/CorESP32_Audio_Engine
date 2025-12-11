// ESP32_UniversalAudio.ino - Beispiel mit SAM Integration

#include "AudioEngine.h"
#include "AudioCodecManager.h"
#include "AudioCodec_SAM.h"

AudioEngine audioEngine;
AudioCodecManager codecManager;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== CorESP32 Audio Engine mit SAM ===");
    
    // Audio Engine initialisieren
    if (!audioEngine.begin()) {
        Serial.println("ERROR: AudioEngine init failed!");
        return;
    }
    
    // Codec Manager initialisieren und SAM registrieren
    codecManager.begin();
    codecManager.registerCodec("sam", new AudioCodec_SAM());
    
    Serial.println("Audio System Ready!");
    Serial.println();
    
    // ========================================================================
    // METHODE 1: Direkt über SAM Codec
    // ========================================================================
    testSAMDirect();
    
    // ========================================================================
    // METHODE 2: Über Codec Manager (wie andere Formate)
    // ========================================================================
    // testSAMViaManager();
    
    // ========================================================================
    // METHODE 3: Mit verschiedenen Voice Presets
    // ========================================================================
    // testSAMVoices();
}

void loop() {
    // Dein normaler Code
    delay(100);
}

// ============================================================================
// METHODE 1: Direkte SAM-Nutzung
// ============================================================================
void testSAMDirect() {
    Serial.println("--- Test: Direct SAM Usage ---");
    
    // SAM Codec erstellen
    AudioCodec_SAM sam;
    
    // Initialisieren
    if (!sam.begin()) {
        Serial.println("SAM init failed!");
        return;
    }
    
    // Voice Preset setzen
    sam.setVoicePreset(SAMVoicePreset::NATURAL);
    
    // Text synthetisieren
    String text = "Hello! This is SAM speech synthesizer running on ESP32.";
    
    if (!sam.synthesizeText(text)) {
        Serial.println("Synthesis failed!");
        return;
    }
    
    Serial.printf("Duration: %u ms\n", sam.getDuration());
    Serial.printf("Samples: %u\n", (sam.getDuration() * sam.getSampleRate()) / 1000);
    
    // Audio lesen und abspielen
    const size_t BUFFER_SIZE = 1024;
    int16_t buffer[BUFFER_SIZE];
    size_t totalRead = 0;
    
    while (sam.isPlaying()) {
        size_t read = sam.read(buffer, BUFFER_SIZE);
        if (read == 0) break;
        
        // HIER: An deine AudioEngine senden
        // audioEngine.write(buffer, read);
        // oder
        // audioEngine.mixerAddSamples(buffer, read);
        
        totalRead += read;
    }
    
    Serial.printf("Played %u samples\n", totalRead);
    Serial.println();
}

// ============================================================================
// METHODE 2: Via Codec Manager (wie MP3/WAV)
// ============================================================================
void testSAMViaManager() {
    Serial.println("--- Test: SAM via CodecManager ---");
    
    // Text als "Datei" behandeln
    String text = "Testing codec manager integration.";
    
    // Codec für "sam" Extension holen
    AudioCodec* codec = codecManager.getCodec("sam");
    if (!codec) {
        Serial.println("SAM codec not found!");
        return;
    }
    
    // "Öffnen" (Text übergeben)
    if (!codec->open(text.c_str())) {
        Serial.println("Failed to open text!");
        return;
    }
    
    // Wie normale Audio-Datei abspielen
    const size_t BUFFER_SIZE = 1024;
    int16_t buffer[BUFFER_SIZE];
    
    while (codec->isPlaying()) {
        size_t read = codec->read(buffer, BUFFER_SIZE);
        if (read == 0) break;
        
        // An AudioEngine senden
        // audioEngine.write(buffer, read);
    }
    
    codec->close();
    Serial.println();
}

// ============================================================================
// METHODE 3: Verschiedene Voice Presets testen
// ============================================================================
void testSAMVoices() {
    Serial.println("--- Test: Different Voice Presets ---");
    
    AudioCodec_SAM sam;
    sam.begin();
    
    const char* presets[] = {"Natural", "Clear", "Warm", "Robot"};
    SAMVoicePreset presetValues[] = {
        SAMVoicePreset::NATURAL,
        SAMVoicePreset::CLEAR,
        SAMVoicePreset::WARM,
        SAMVoicePreset::ROBOT
    };
    
    for (int i = 0; i < 4; i++) {
        Serial.printf("Testing preset: %s\n", presets[i]);
        
        sam.setVoicePreset(presetValues[i]);
        sam.synthesizeText("This is a voice test.");
        
        // Abspielen...
        const size_t BUFFER_SIZE = 1024;
        int16_t buffer[BUFFER_SIZE];
        
        while (sam.isPlaying()) {
            size_t read = sam.read(buffer, BUFFER_SIZE);
            if (read == 0) break;
            // audioEngine.write(buffer, read);
        }
        
        delay(500); // Pause zwischen Tests
    }
    
    Serial.println();
}

// ============================================================================
// METHODE 4: Custom Voice Parameters
// ============================================================================
void testCustomVoice() {
    Serial.println("--- Test: Custom Voice Parameters ---");
    
    AudioCodec_SAM sam;
    sam.begin();
    
    // Eigene Parameter setzen
    SAMVoiceParams params;
    params.speed = 80;          // Schneller
    params.pitch = 75;          // Höher
    params.throat = 120;        // Enger
    params.mouth = 130;         // Offener
    params.smoothing = 50;      // Mehr Glättung
    params.interpolation = 60;  // Mehr Interpolation
    params.formantBoost = 25;   // Mehr Formant-Boost
    params.bassBoost = 3;       // +3 dB Bass
    
    sam.setVoiceParams(params);
    sam.synthesizeText("Custom voice test with modified parameters.");
    
    // Abspielen...
    Serial.println();
}

// ============================================================================
// METHODE 5: Direkt in Buffer synthetisieren (für Effekte, etc.)
// ============================================================================
void testDirectBuffer() {
    Serial.println("--- Test: Direct Buffer Synthesis ---");
    
    AudioCodec_SAM sam;
    sam.begin();
    
    // Großer Buffer
    const size_t MAX_SAMPLES = 50000;
    int16_t* buffer = (int16_t*)malloc(MAX_SAMPLES * sizeof(int16_t));
    
    if (buffer) {
        size_t actualSamples = 0;
        
        if (sam.synthesizeTextToBuffer(
            "Direct buffer synthesis test.", 
            buffer, 
            MAX_SAMPLES, 
            &actualSamples
        )) {
            Serial.printf("Generated %u samples\n", actualSamples);
            
            // Jetzt kannst du Effekte anwenden, etc.
            // applyReverb(buffer, actualSamples);
            // applyEcho(buffer, actualSamples);
            
            // Und dann abspielen
            // audioEngine.write(buffer, actualSamples);
        }
        
        free(buffer);
    }
    
    Serial.println();
}

// ============================================================================
// METHODE 6: Console-Commands für SAM
// ============================================================================
void handleConsoleCommand(const String& cmd) {
    if (cmd.startsWith("speak ")) {
        String text = cmd.substring(6);
        
        AudioCodec_SAM sam;
        sam.begin();
        sam.synthesizeText(text);
        
        // Abspielen...
        const size_t BUFFER_SIZE = 1024;
        int16_t buffer[BUFFER_SIZE];
        
        while (sam.isPlaying()) {
            size_t read = sam.read(buffer, BUFFER_SIZE);
            if (read == 0) break;
            // audioEngine.write(buffer, read);
        }
    }
    else if (cmd.startsWith("voice ")) {
        String preset = cmd.substring(6);
        
        // Preset wechseln...
        Serial.printf("Voice preset changed to: %s\n", preset.c_str());
    }
}