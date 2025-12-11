# SAM Speech Synthesis Engine for ESP32
## Modern, Native Implementation - No C64 Legacy

---

## 🎯 Overview

This is a **complete rewrite** of the SAM (Software Automatic Mouth) speech synthesizer, specifically optimized for the ESP32 microcontroller. **All C64 legacy code has been eliminated** and replaced with modern, ESP32-native implementations that fully utilize the hardware capabilities.

### Key Features

✅ **ESP32-Native Architecture**
- 32-bit FPU-optimized arithmetic (no 8-bit integer limitations)
- Dual-core processing (synthesis on Core 1, text processing on Core 0)
- DMA-based I2S audio output
- PSRAM support for large buffers

✅ **Modern Signal Processing**
- Scientifically accurate formant synthesis
- Advanced DSP: smoothing, cubic interpolation, formant boost
- Biquad filters for EQ
- Real-time parameter modulation

✅ **Maximum Customization**
- JSON-based configuration
- Multiple voice presets (Natural, Clear, Warm, Robot, Child, Deep)
- Real-time parameter adjustment
- Custom pronunciation dictionary

✅ **Professional Code Quality**
- Clean C++ architecture
- Thread-safe operation
- Comprehensive error handling
- Debug and diagnostics

---

## 📁 File Structure

```
CorESP32_Audio_Engine/
├── SAMEngine.h              // Main engine header
├── SAMEngine.cpp            // Implementation (parts 1-3)
├── SAMPhonemes.h            // Modern phoneme definitions
├── SAMPhonemes.cpp          // Phoneme tables
├── SAMDSPProcessor.h        // DSP enhancement
├── SAMDSPProcessor.cpp      // DSP implementation
└── data/
    └── sam_config.json      // Configuration file
```

---

## 🚀 Integration Guide

### Step 1: Add Files to Your Project

Copy all SAM files to your CorESP32_Audio_Engine project:

```bash
# Copy headers
cp SAMEngine.h SAMPhonemes.h SAMDSPProcessor.h /your/project/

# Copy implementations
cp SAMEngine.cpp SAMPhonemes.cpp SAMDSPProcessor.cpp /your/project/

# Copy configuration
cp sam_config.json /your/project/data/
```

### Step 2: Update Your Main Sketch

```cpp
#include "AudioEngine.h"
#include "SAMEngine.h"

AudioEngine audioEngine;
SAMEngine samEngine;

void setup() {
    Serial.begin(115200);
    
    // Initialize SPIFFS for configuration
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }
    
    // Initialize Audio Engine
    if (!audioEngine.begin()) {
        Serial.println("AudioEngine initialization failed!");
        return;
    }
    
    // Initialize SAM Engine
    if (!samEngine.begin(&audioEngine)) {
        Serial.println("SAMEngine initialization failed!");
        return;
    }
    
    // Load configuration from JSON
    samEngine.loadConfig("/sam_config.json");
    
    // Apply a preset
    samEngine.applyPreset(SAMVoicePreset::NATURAL);
    
    // Optional: Set callbacks
    samEngine.onSpeechStart([]() {
        Serial.println("Speech started");
    });
    
    samEngine.onSpeechComplete([]() {
        Serial.println("Speech completed");
    });
    
    // Speak!
    samEngine.speak("Hello, this is SAM running natively on ESP32!", true);
}

void loop() {
    // Your application code
    delay(10);
}
```

### Step 3: Upload Configuration to SPIFFS

Use Arduino IDE or PlatformIO to upload the `data` folder:

**Arduino IDE:**
- Tools → ESP32 Sketch Data Upload

**PlatformIO:**
```bash
pio run --target uploadfs
```

---

## 🎨 Voice Configuration

### Using Presets

```cpp
// Natural-sounding voice
samEngine.applyPreset(SAMVoicePreset::NATURAL);

// Clear articulation
samEngine.applyPreset(SAMVoicePreset::CLEAR);

// Warm, slow voice
samEngine.applyPreset(SAMVoicePreset::WARM);

// Classic robot sound
samEngine.applyPreset(SAMVoicePreset::ROBOT);
```

### Custom Parameters

```cpp
SAMVoiceParams params;
params.speed = 70;          // 40-150 (72 = normal)
params.pitch = 65;          // 20-120 (64 = normal)
params.throat = 135;        // 90-180 (128 = normal)
params.mouth = 140;         // 90-180 (128 = normal)
params.smoothing = 40;      // 0-70
params.interpolation = 45;  // 0-80
params.formantBoost = 20;   // 0-50
params.bassBoost = 2;       // -10 to +10 dB
params.enableProsody = true;

samEngine.setVoiceParams(params);
```

### Real-time Modulation

```cpp
// Add vibrato
samEngine.setPitchModulation([](float time) {
    return 1.0f + 0.15f * sin(time * 4.0f * PI);
});

// Variable speed
samEngine.setSpeedModulation([](float time) {
    return 1.0f + 0.1f * sin(time * 2.0f * PI);
});
```

---

## 🔧 JSON Configuration

Edit `sam_config.json` to customize behavior:

```json
{
  "samEngine": {
    "audio": {
      "sampleRate": 22050,
      "bufferSize": 8192,
      "usePSRAM": true
    },
    "presets": {
      "natural": {
        "speed": 68,
        "pitch": 70,
        "smoothing": 40,
        ...
      }
    },
    "dsp": {
      "smoothing": {"enabled": true},
      "interpolation": {"algorithm": "cubic"},
      ...
    }
  }
}
```

---

## 📊 API Reference

### Main Functions

```cpp
// Initialization
bool begin(AudioEngine* audioEngine);
void end();

// Configuration
bool loadConfig(const char* jsonPath = "/sam_config.json");
bool saveConfig(const char* jsonPath = "/sam_config.json");

// Voice control
void setVoiceParams(const SAMVoiceParams& params);
void applyPreset(SAMVoicePreset preset);

// Speech synthesis
bool speak(const String& text, bool async = true);
bool speakPhonemes(const String& phonemeString, bool async = true);
void stop();
bool isSpeaking() const;

// Audio buffer generation (for custom playback)
size_t generateBuffer(const String& text, int16_t* buffer, 
                      size_t maxSamples, uint32_t sampleRate = 22050);
float* generateFloatBuffer(const String& text, size_t* outLength, 
                           uint32_t sampleRate = 22050);

// Callbacks
void onSpeechStart(std::function<void()> callback);
void onSpeechComplete(std::function<void()> callback);
void onPhonemeChange(std::function<void(const Phoneme&)> callback);

// Real-time modulation
void setPitchModulation(std::function<float(float)> modulator);
void setSpeedModulation(std::function<float(float)> modulator);

// Debug
void setDebugMode(bool enable);
void printPhonemeSequence(const String& text);
void printFormantData(const Phoneme& phoneme);
```

---

## 🎯 What's Different from C64 SAM?

| Aspect | C64 SAM | ESP32-Native SAM |
|--------|---------|------------------|
| **Arithmetic** | 8-bit integer | 32-bit float (FPU) |
| **Sample Rate** | 8 kHz | 22-44 kHz |
| **Bit Depth** | 4-bit | 16-bit |
| **Processing** | Single-threaded, blocking | Multi-core, async |
| **Formants** | LUT-based approximations | Real-time calculation |
| **Interpolation** | Linear (rough) | Cubic/Sinc |
| **Phonemes** | C64 tables | IPA-based, scientifically accurate |
| **Memory** | 64KB total | MB PSRAM available |
| **DSP** | None | Smoothing, EQ, compression, reverb |

---

## 🧪 Testing & Debugging

### Enable Debug Mode

```cpp
samEngine.setDebugMode(true);
```

### Test Phoneme Sequence

```cpp
samEngine.printPhonemeSequence("Hello world");
```

Output:
```
=== Phoneme Sequence ===
Text: Hello world
Total phonemes: 12
Total duration: 1.45 s

  0: CONSONANT_F  idx= 58 dur= 70 ms stress=0.65
  1: VOWEL        idx= 12 dur=143 ms stress=0.65 [WORD]
  2: CONSONANT_L  idx= 80 dur= 99 ms stress=0.50
  ...
```

### Monitor Statistics

```cpp
SAMEngine::Stats stats = samEngine.getStats();
Serial.printf("Total synthesized: %u\n", stats.totalSynthesized);
Serial.printf("Avg CPU load: %.2f%%\n", stats.avgCPULoad);
```

---

## 🎓 Advanced Usage

### Custom Pronunciation Dictionary

```cpp
// Add custom word pronunciations
SAMTextRules::addDictionaryEntry("Arduino", "AARDUWIYOW");
SAMTextRules::addDictionaryEntry("ESP32", "IY EH S PIY THERTIY TUW");
```

### Phoneme Change Callback

```cpp
samEngine.onPhonemeChange([](const Phoneme& ph) {
    // Visualize current phoneme
    Serial.printf("Phoneme: %d, Duration: %d ms\n", 
                 ph.index, ph.duration_ms);
    
    // Control LEDs based on vowel/consonant
    if (ph.type == PhonemeType::VOWEL) {
        digitalWrite(LED_VOWEL, HIGH);
    }
});
```

### Generate WAV File

```cpp
// Generate audio to buffer
size_t samples;
float* buffer = samEngine.generateFloatBuffer("Hello", &samples);

// Save as WAV file (implement your own WAV writer)
saveWAV("/spiffs/hello.wav", buffer, samples, 22050);

free(buffer);
```

---

## 📝 TODO / Future Enhancements

- [ ] Complete text-to-phoneme rule system (CMU pronunciation dictionary)
- [ ] SSML support for advanced prosody control
- [ ] Multiple voices (male/female/child parameters)
- [ ] Emotion control (happy, sad, angry, etc.)
- [ ] Real-time formant visualization
- [ ] Integration with your AudioEngine's mixer
- [ ] Singing mode
- [ ] Network streaming support

---

## 🐛 Troubleshooting

### "SPIFFS initialization failed"
- Make sure to upload the `data` folder with sam_config.json
- Check `SPIFFS.begin(true)` is called before SAM initialization

### "Out of memory"
- Enable PSRAM in your board configuration
- Reduce `bufferSize` in sam_config.json
- Check `psramFound()` returns true

### "Synthesis failed"
- Enable debug mode: `samEngine.setDebugMode(true)`
- Check Serial output for detailed error messages
- Verify text is not empty

### Audio quality issues
- Increase sample rate to 44100 Hz
- Adjust DSP parameters (smoothing, interpolation)
- Fine-tune voice parameters (throat, mouth)

---

## 📚 References

- Original SAM: https://github.com/s-macke/SAM
- Formant synthesis: "Speech and Language Processing" by Jurafsky & Martin
- Peterson & Barney (1952): "Control Methods Used in a Study of the Vowels"
- ESP32 Documentation: https://docs.espressif.com/

---

## 📄 License

This implementation is based on the original SAM speech synthesizer but has been completely rewritten for ESP32. 

**No C64 legacy code remains - this is a modern, from-scratch implementation.**

---

## 💬 Support

For issues, questions, or contributions:
- Open an issue in the repository
- Check the debug output
- Review the JSON configuration

---

**Happy Synthesizing! 🎙️**
