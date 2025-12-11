// SAMEngine.h - Modern SAM Speech Synthesizer for ESP32
// No C64 legacy code - Pure ESP32 optimization
// Integrated with CorESP32_Audio_Engine

#ifndef SAM_ENGINE_H
#define SAM_ENGINE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "AudioEngine.h"
#include "AudioConfig.h"

// Forward declarations
struct SAMConfig;
struct PhonemeSequence;
struct FormantFrame;
class SAMDSPProcessor;

// ============================================================================
// SAM Voice Configuration
// ============================================================================

struct SAMVoiceParams {
    // Core SAM parameters (0-255 range for compatibility)
    uint8_t speed;      // 40-150, default 72
    uint8_t pitch;      // 20-120, default 64
    uint8_t throat;     // 90-180, default 128 (affects formant frequencies)
    uint8_t mouth;      // 90-180, default 128 (affects formant amplitudes)
    
    // DSP Enhancement parameters (0-100 range)
    uint8_t smoothing;     // 0-70, default 35
    uint8_t interpolation; // 0-80, default 40
    uint8_t formantBoost;  // 0-50, default 15
    int8_t bassBoost;      // -10 to +10 dB, default 0
    
    // Advanced parameters
    float pitchVariance;   // 0.0-1.0, adds natural pitch variation
    float speedVariance;   // 0.0-1.0, adds natural speed variation
    bool enableProsody;    // Enable prosodic features
    
    SAMVoiceParams() :
        speed(72), pitch(64), throat(128), mouth(128),
        smoothing(35), interpolation(40), formantBoost(15), bassBoost(0),
        pitchVariance(0.1f), speedVariance(0.05f), enableProsody(true) {}
};

// Preset voice profiles
enum class SAMVoicePreset {
    NATURAL,    // Balanced, natural-sounding
    CLEAR,      // Clear articulation
    WARM,       // Warm, slower, deeper
    ROBOT,      // Classic robotic sound
    CUSTOM      // User-defined
};

// ============================================================================
// Phoneme System (Modern IPA-inspired, not C64)
// ============================================================================

enum class PhonemeType : uint8_t {
    SILENCE,
    VOWEL,
    CONSONANT_STOP,      // p, b, t, d, k, g
    CONSONANT_FRICATIVE, // f, v, s, z, sh, etc.
    CONSONANT_NASAL,     // m, n, ng
    CONSONANT_LIQUID,    // l, r
    CONSONANT_GLIDE      // w, y
};

struct Phoneme {
    PhonemeType type;
    uint8_t index;           // Index into formant tables
    uint16_t duration_ms;    // Duration in milliseconds
    float stress;            // 0.0-1.0, affects amplitude and duration
    bool wordBoundary;       // Marks word boundary for prosody
    
    Phoneme() : type(PhonemeType::SILENCE), index(0), 
                duration_ms(100), stress(0.5f), wordBoundary(false) {}
};

// ============================================================================
// Formant Structure (ESP32-optimized, using FPU)
// ============================================================================

struct FormantSet {
    float f1, f2, f3;        // Formant frequencies (Hz)
    float a1, a2, a3;        // Formant amplitudes (0.0-1.0)
    float bw1, bw2, bw3;     // Formant bandwidths (Hz)
    
    FormantSet() : f1(500), f2(1500), f3(2500),
                   a1(1.0f), a2(0.5f), a3(0.3f),
                   bw1(100), bw2(120), bw3(150) {}
    
    // Interpolate between two formant sets
    FormantSet interpolate(const FormantSet& target, float t) const;
};

// ============================================================================
// Main SAM Engine Class
// ============================================================================

class SAMEngine {
public:
    SAMEngine();
    ~SAMEngine();
    
    // ========================================================================
    // Initialization & Configuration
    // ========================================================================
    
    bool begin(AudioEngine* audioEngine);
    void end();
    
    // Load configuration from JSON file
    bool loadConfig(const char* jsonPath = "/sam_config.json");
    bool saveConfig(const char* jsonPath = "/sam_config.json");
    
    // Voice parameter control
    void setVoiceParams(const SAMVoiceParams& params);
    SAMVoiceParams getVoiceParams() const { return m_voiceParams; }
    
    // Apply preset
    void applyPreset(SAMVoicePreset preset);
    
    // ========================================================================
    // Speech Synthesis
    // ========================================================================
    
    // Main synthesis functions
    bool speak(const String& text, bool async = true);
    bool speakPhonemes(const String& phonemeString, bool async = true);
    
    // Queue management
    void stop();
    bool isSpeaking() const { return m_isSpeaking; }
    void clearQueue();
    
    // Callbacks
    typedef std::function<void()> SpeechCallback;
    void onSpeechStart(SpeechCallback callback) { m_onSpeechStart = callback; }
    void onSpeechComplete(SpeechCallback callback) { m_onSpeechComplete = callback; }
    void onPhonemeChange(std::function<void(const Phoneme&)> callback) { 
        m_onPhonemeChange = callback; 
    }
    
    // ========================================================================
    // Real-time Parameter Modulation
    // ========================================================================
    
    void setPitchModulation(std::function<float(float)> modulator);
    void setSpeedModulation(std::function<float(float)> modulator);
    
    // ========================================================================
    // Audio Buffer Generation (for external use)
    // ========================================================================
    
    // Generate audio buffer from text
    size_t generateBuffer(const String& text, int16_t* buffer, 
                          size_t maxSamples, uint32_t sampleRate = 22050);
    
    // Generate and return as float array
    float* generateFloatBuffer(const String& text, size_t* outLength, 
                               uint32_t sampleRate = 22050);
    
    // ========================================================================
    // Debug & Diagnostics
    // ========================================================================
    
    void setDebugMode(bool enable) { m_debugMode = enable; }
    void printPhonemeSequence(const String& text);
    void printFormantData(const Phoneme& phoneme);
    
    // Statistics
    struct Stats {
        uint32_t totalSynthesized;
        uint32_t totalDuration_ms;
        float avgCPULoad;
        uint32_t bufferUnderruns;
    };
    Stats getStats() const { return m_stats; }
    void resetStats();

private:
    // ========================================================================
    // Internal Core Functions
    // ========================================================================
    
    // Text processing pipeline
    PhonemeSequence textToPhonemes(const String& text);
    void convertWordToPhonemes(const String& word, PhonemeSequence& sequence);
    void applyProsody(PhonemeSequence& sequence);
    void applySentenceIntonation(PhonemeSequence& sequence, size_t start, size_t end);
    void calculateDurations(PhonemeSequence& sequence);
    void calculateStress(PhonemeSequence& sequence);
    
    // Synthesis pipeline
    size_t synthesizePhonemes(const PhonemeSequence& sequence, 
                             float* buffer, size_t maxSamples);
    void generateFormantFrame(const Phoneme& phoneme, float t, 
                             FormantSet& output);
    void renderFrame(const FormantSet& formants, float* output, 
                    size_t frameSize);
    
    // DSP processing
    void applySmoothing(float* buffer, size_t length);
    void applyInterpolation(float* buffer, size_t length);
    void applyFormantBoost(float* buffer, size_t length);
    void applyBassBoost(float* buffer, size_t length);
    
    // Multi-core synthesis task
    static void synthTaskFunc(void* parameter);
    void synthTask();
    
    // Formant synthesis (FPU-optimized)
    float generateFormantSample(float frequency, float amplitude, 
                               float bandwidth, float phase);
    void updateOscillatorPhases(float deltaTime);
    
    // ========================================================================
    // Member Variables
    // ========================================================================
    
    AudioEngine* m_audioEngine;
    SAMVoiceParams m_voiceParams;
    SAMVoicePreset m_currentPreset;
    
    // Synthesis state
    volatile bool m_isSpeaking;
    bool m_debugMode;
    
    // Multi-core task
    TaskHandle_t m_synthTask;
    QueueHandle_t m_textQueue;
    SemaphoreHandle_t m_mutex;
    
    // Audio buffer (PSRAM if available)
    float* m_audioBuffer;
    size_t m_audioBufferSize;
    static constexpr size_t DEFAULT_BUFFER_SIZE = 8192;
    
    // Oscillator state (for formant generation)
    struct OscillatorState {
        float phase1, phase2, phase3;
        float lastFreq1, lastFreq2, lastFreq3;
    } m_oscState;
    
    // DSP processor
    SAMDSPProcessor* m_dspProcessor;
    
    // Callbacks
    SpeechCallback m_onSpeechStart;
    SpeechCallback m_onSpeechComplete;
    std::function<void(const Phoneme&)> m_onPhonemeChange;
    
    // Modulation functions
    std::function<float(float)> m_pitchModulator;
    std::function<float(float)> m_speedModulator;
    
    // Statistics
    Stats m_stats;
    
    // Configuration
    static constexpr uint32_t SAM_SAMPLE_RATE = 22050;
    static constexpr size_t MAX_TEXT_QUEUE_SIZE = 10;
};

// ============================================================================
// Helper Structures
// ============================================================================

struct PhonemeSequence {
    std::vector<Phoneme> phonemes;
    float totalDuration_ms;
    
    PhonemeSequence() : totalDuration_ms(0) {}
};

// ============================================================================
// Global Preset Definitions
// ============================================================================

namespace SAMPresets {
    SAMVoiceParams getNatural();
    SAMVoiceParams getClear();
    SAMVoiceParams getWarm();
    SAMVoiceParams getRobot();
}

#endif // SAM_ENGINE_H