# SAM Speech Synthesis - ESP32 Native Redesign

## Ziel: C64-Altlasten eliminieren, ESP32-Hardware optimal nutzen

### Probleme der bisherigen Portierungen

**C64-Altlasten:**
- 8-Bit-Arithmetik auf 32-Bit-System
- Keine Nutzung von FPU
- Kein SIMD/Vektorisierung
- Statische, nicht-konfigurierbare Tabellen
- Ineffizientes Memory-Layout
- Keine Nutzung von DMA
- Blockierendes Processing

**ESP32-Potenzial wird nicht genutzt:**
- Dual-Core Xtensa LX6/LX7
- Hardware-FPU
- SIMD-Instruktionen
- DSP-Beschleunigung
- I2S mit DMA
- PSRAM für große Buffers
- FreeRTOS für echtes Multithreading

---

## Moderne ESP32-Native Architektur

### 1. Phonem-Engine (Core 1)

```cpp
class PhonemEngine {
private:
    // Nutze FPU für alle Berechnungen
    float formant_frequencies[3];  // F1, F2, F3
    float formant_amplitudes[3];
    float formant_bandwidths[3];
    
    // SIMD-optimierte Tabellen (aligned für ESP32)
    alignas(16) float sine_table[256];
    alignas(16) float formant_lut[PHONEME_COUNT][3];
    
    // Hardware-Timer für präzises Timing
    hw_timer_t* sample_timer;
    
public:
    // Phonem-zu-Parameter Mapping (neu berechnet, nicht C64-Tabellen)
    struct PhonemParams {
        float duration_ms;
        float pitch_hz;
        float stress;
        FormantSet formants;
        TransitionCurve transition;
    };
    
    // Interpolation mit FPU statt Integer-Arithmetik
    void interpolateFormants(
        const PhonemParams& from,
        const PhonemParams& to,
        float t,  // 0.0 - 1.0
        FormantSet& output
    );
};
```

### 2. Formant-Synthesizer (optimiert)

```cpp
class FormantSynthesizer {
private:
    // Parallele Formant-Generierung
    struct FormantOscillator {
        float phase;
        float freq;
        float amp;
        float bw;
        
        // Biquad-Filter statt C64-Approximation
        BiquadFilter filter;
    };
    
    FormantOscillator oscillators[3];  // F1, F2, F3
    
    // DMA-Buffer für I2S
    int16_t* dma_buffer;
    size_t dma_buffer_size;
    
public:
    // SIMD-optimierte Sample-Generierung
    void generateSamples(float* output, size_t count) {
        // Nutze ESP32 SIMD wo möglich
        for (size_t i = 0; i < count; i += 4) {
            // 4 Samples parallel verarbeiten
            __asm__ volatile (
                // ESP32-spezifische SIMD-Instruktionen
            );
        }
    }
    
    // Nicht-blockierend dank DMA
    void streamToI2S(i2s_port_t port);
};
```

### 3. Text-zu-Phonem (moderne Regeln)

```cpp
class TextToPhonemeConverter {
private:
    // Moderne C++-Strukturen statt C64-Arrays
    std::unordered_map<std::string, std::vector<Phoneme>> dictionary;
    std::vector<Rule> pronunciation_rules;
    
    // Kontextbasierte Analyse
    struct PhonemeContext {
        Phoneme prev;
        Phoneme current;
        Phoneme next;
        float stress_level;
        bool word_boundary;
    };
    
public:
    // Neue Regel-Engine (nicht C64-basiert)
    std::vector<Phoneme> textToPhonemes(const std::string& text);
    
    // Prosody-Modell
    void applyProsody(std::vector<Phoneme>& phonemes);
    void calculateStress(std::vector<Phoneme>& phonemes);
    void applyIntonation(std::vector<Phoneme>& phonemes);
};
```

### 4. Parameter-Tabellen (neu berechnet für ESP32)

**WICHTIG:** Keine C64-Tabellen übernehmen!

```cpp
// Moderne, physikalisch basierte Formant-Werte
constexpr FormantData VOWEL_FORMANTS[] = {
    // Basierend auf akustischen Messungen, nicht C64-Approximationen
    {"a", {730.0f, 1090.0f, 2440.0f}, {1.0f, 0.5f, 0.3f}},  // /a/
    {"e", {530.0f, 1840.0f, 2480.0f}, {1.0f, 0.4f, 0.25f}}, // /e/
    {"i", {270.0f, 2290.0f, 3010.0f}, {1.0f, 0.35f, 0.2f}}, // /i/
    {"o", {570.0f, 840.0f, 2410.0f}, {1.0f, 0.45f, 0.28f}},  // /o/
    {"u", {300.0f, 870.0f, 2240.0f}, {1.0f, 0.4f, 0.25f}},   // /u/
};

// Konsonanten mit realistischen Noise-Spektren
constexpr ConsonantData CONSONANT_PARAMS[] = {
    // Keine C64-Werte, sondern echte spektrale Analysen
    {"s", NoiseType::FRICATIVE, {4000.0f, 8000.0f}, 0.8f},
    {"sh", NoiseType::FRICATIVE, {2500.0f, 6000.0f}, 0.7f},
    {"f", NoiseType::FRICATIVE, {5000.0f, 10000.0f}, 0.6f},
};
```

### 5. Multi-Core Architektur

```cpp
class SAMEngine {
private:
    // Core 0: Text-Processing & Control
    TaskHandle_t text_task;
    QueueHandle_t phoneme_queue;
    
    // Core 1: Audio-Synthesis (Echtzeit)
    TaskHandle_t audio_task;
    RingBuffer<float> audio_buffer;
    
public:
    void initialize() {
        // Core 0: Text-zu-Phonem
        xTaskCreatePinnedToCore(
            textProcessingTask,
            "TextProc",
            8192,
            this,
            1,  // Niedrigere Priorität
            &text_task,
            0   // Core 0
        );
        
        // Core 1: Audio-Synthesis (Echtzeit)
        xTaskCreatePinnedToCore(
            audioSynthesisTask,
            "AudioSynth",
            8192,
            this,
            10, // Hohe Priorität
            &audio_task,
            1   // Core 1 - dediziert für Audio
        );
    }
    
    // Nicht-blockierende API
    void speakAsync(const std::string& text);
    bool isSpeaking() const;
    void stop();
};
```

### 6. Qualitätsverbesserungen

**Statt C64-Limitierungen:**

```cpp
class AudioQuality {
public:
    // Moderne Sample-Rate (nicht 8kHz vom C64)
    static constexpr uint32_t SAMPLE_RATE = 22050;  // Oder 44100
    
    // 16-Bit statt 4-Bit
    static constexpr uint32_t BIT_DEPTH = 16;
    
    // Interpolation zwischen Phonemen
    enum class InterpolationMode {
        LINEAR,
        CUBIC,
        SINC  // Beste Qualität
    };
    
    // Dynamische Formant-Anpassung
    void adjustForPitch(float pitch_factor);
    void adjustForSpeed(float speed_factor);
    
    // Rauschgenerierung (nicht C64-LFSR)
    float generateNoise() {
        // Echter White-Noise-Generator
        static std::mt19937 rng;
        static std::normal_distribution<float> dist(0.0f, 1.0f);
        return dist(rng);
    }
};
```

### 7. I2S-Integration (DMA)

```cpp
class I2SAudioOutput {
private:
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 22050,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = true,  // Präzise Clock
        .tx_desc_auto_clear = true
    };
    
public:
    // Zero-Copy wo möglich
    void writeSamples(const int16_t* samples, size_t count) {
        size_t bytes_written;
        i2s_write(I2S_NUM_0, samples, count * sizeof(int16_t), 
                  &bytes_written, portMAX_DELAY);
    }
    
    // Buffer-Callback für kontinuierliches Streaming
    void setBufferCallback(std::function<void(int16_t*, size_t)> callback);
};
```

### 8. Performance-Optimierungen

```cpp
class PerformanceOptimizer {
public:
    // FPU-optimierte DSP-Funktionen
    static inline float fastSin(float x) {
        // Nutze ESP32 FPU-Instruktionen
        float result;
        __asm__ volatile (
            "sin.s %0, %1"
            : "=f" (result)
            : "f" (x)
        );
        return result;
    }
    
    // SIMD für Formant-Mixing
    static void mixFormants(
        const float* f1,
        const float* f2,
        const float* f3,
        float* output,
        size_t count
    ) {
        // ESP32 SIMD: 4 Samples parallel
        for (size_t i = 0; i < count; i += 4) {
            // Vektorisierte Addition
        }
    }
    
    // Cache-optimiertes Layout
    alignas(16) struct alignas(16) CacheOptimized {
        float data[4];  // Passt in eine Cache-Line
    };
};
```

---

## Implementierungs-Roadmap

### Phase 1: Grundlagen (C64-frei)
1. ✅ Neue Phonem-Definitionen basierend auf IPA
2. ✅ FPU-basierte Formant-Berechnung
3. ✅ I2S-DMA Integration
4. ✅ Multi-Core-Architektur

### Phase 2: Qualität
1. ⏳ Hochwertige Interpolation
2. ⏳ Prosody-Modell
3. ⏳ Dynamische Anpassungen
4. ⏳ Noise-Shaping für Konsonanten

### Phase 3: Features
1. ⏳ Emotionen/Stimmungen
2. ⏳ Stimm-Parameter (männlich/weiblich/Kind)
3. ⏳ Mehrstimmigkeit
4. ⏳ SSML-Support

### Phase 4: Optimierung
1. ⏳ SIMD-Vektorisierung
2. ⏳ Zero-Copy Buffers
3. ⏳ Echtzeitfähigkeit garantieren
4. ⏳ Power-Optimierung

---

## Kritische Unterschiede zu C64-SAM

| Aspekt | C64-SAM | ESP32-Native |
|--------|---------|--------------|
| Arithmetik | 8-Bit Integer | 32-Bit Float (FPU) |
| Sample-Rate | 8 kHz | 22-44 kHz |
| Bit-Tiefe | 4-Bit | 16-Bit |
| Processing | Blockierend | Multi-Core + DMA |
| Formanten | LUT-basiert | Echtzeit-Berechnung |
| Interpolation | Linear (grob) | Sinc/Cubic |
| Phoneme | C64-Tabellen | IPA-basiert |
| Noise | LFSR | True Random/Gaussian |
| Memory | 64KB gesamt | MB PSRAM |

---

## Verwendung

```cpp
// Initialisierung
SAMEngine sam;
sam.initialize();
sam.setVoice(VoiceProfile::MALE_ADULT);
sam.setSpeed(1.0f);  // Normal
sam.setPitch(1.0f);  // Normal

// Nicht-blockierend sprechen
sam.speakAsync("Hello, I am SAM running natively on ESP32!");

// Warten bis fertig
while (sam.isSpeaking()) {
    vTaskDelay(pdMS_TO_TICKS(10));
}

// Mit Callbacks
sam.onSpeechComplete([]() {
    Serial.println("Speech finished!");
});

// Echtzeit-Parameter-Änderung
sam.setPitchDynamic([](float time) {
    return 1.0f + 0.2f * sinf(time * 2.0f * PI);  // Vibrato
});
```

---

## Nächste Schritte

1. **Phonem-Database neu erstellen**
   - Keine C64-Werte übernehmen
   - Moderne Formant-Messungen verwenden
   - IPA-Standard befolgen

2. **FPU-basierte Formant-Synthese**
   - Keine Integer-Approximationen
   - Echte Biquad-Filter
   - SIMD wo möglich

3. **DMA-Integration**
   - Zero-Copy Architektur
   - Ping-Pong Buffering
   - Jitter-freie Ausgabe

4. **Qualitätssicherung**
   - A/B-Tests gegen moderne TTS
   - Spektralanalyse
   - Verständlichkeitstests

**Ziel:** Eine