/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM SPEECH SYNTHESIS ENGINE - Header                                       ║
 ║  Modern ESP32 Implementation - No C64 Legacy Code                           ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#ifndef SAM_ENGINE_H
#define SAM_ENGINE_H

#include <Arduino.h>
#include <vector>

// Forward declarations
class AudioEngine;
class SAMDSPProcessor;

// ═══════════════════════════════════════════════════════════════════════════
// Voice Presets
// ═══════════════════════════════════════════════════════════════════════════

enum class SAMVoicePreset : uint8_t {
    NATURAL = 0,  // Balanced, natural-sounding voice
    CLEAR,        // Enhanced clarity, slightly higher pitch
    WARM,         // Warm, deeper voice
    ROBOT,        // Robotic, monotone
    CHILD,        // Higher pitched, faster
    DEEP          // Deep male voice
};

// ═══════════════════════════════════════════════════════════════════════════
// Data Structures
// ═══════════════════════════════════════════════════════════════════════════

struct FormantSet {
    float f1_freq, f2_freq, f3_freq;    // Formant frequencies
    float f1_amp, f2_amp, f3_amp;       // Formant amplitudes
    float f1_bw, f2_bw, f3_bw;          // Formant bandwidths
    
    FormantSet() : f1_freq(0), f2_freq(0), f3_freq(0),
                   f1_amp(0), f2_amp(0), f3_amp(0),
                   f1_bw(0), f2_bw(0), f3_bw(0) {}
};

struct Phoneme {
    char symbol[4];
    uint8_t duration;
    uint8_t pitch;
    uint8_t amplitude;
    bool voiced;
    FormantSet formants;
};

struct PhonemeSequence {
    std::vector<Phoneme> phonemes;
    uint32_t totalDuration;
};

struct SAMVoiceParams {
    uint8_t speed;      // 50-150 (default: 72)
    uint8_t pitch;      // 0-255 (default: 64)
    uint8_t throat;     // 0-255 (default: 128)
    uint8_t mouth;      // 0-255 (default: 128)
    uint8_t stress;     // 0-255 (default: 0)
    
    SAMVoiceParams() 
        : speed(72), pitch(64), throat(128), mouth(128), stress(0) {}
};

struct SAMConfig {
    bool enableDSP;
    bool enableSmoothing;
    bool enableInterpolation;
    float smoothingAmount;
    float interpolationAmount;
    bool enableFormantBoost;
    bool enableBassBoost;
    
    SAMConfig()
        : enableDSP(true)
        , enableSmoothing(true)
        , enableInterpolation(true)
        , smoothingAmount(0.3f)
        , interpolationAmount(0.5f)
        , enableFormantBoost(true)
        , enableBassBoost(false) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// SAM Engine Class
// ═══════════════════════════════════════════════════════════════════════════

class SAMEngine {
public:
    SAMEngine();
    ~SAMEngine();
    
    // Initialization
    bool begin(AudioEngine* engine = nullptr);
    void end();
    
    // Configuration
    bool loadConfig(const char* jsonPath);
    bool saveConfig(const char* jsonPath);
    void setVoiceParams(const SAMVoiceParams& params);
    SAMVoiceParams getVoiceParams() const { return m_voiceParams; }
    void setConfig(const SAMConfig& config);
    SAMConfig getConfig() const { return m_config; }
    
    // Presets
    void applyPreset(SAMVoicePreset preset);
    
    // Synthesis (main API)
    bool speak(const String& text, bool async = false);
    size_t synthesize(const PhonemeSequence& sequence, std::vector<float>& output);
    
    // Text processing
    PhonemeSequence textToPhonemes(const String& text);
    void applyProsody(PhonemeSequence& sequence);
    
    // Status
    bool isSpeaking() const { return m_isSpeaking; }
    float getProgress() const { return m_progress; }
    
private:
    // Audio engine (optional)
    AudioEngine* m_audioEngine;
    
    // DSP Processor
    SAMDSPProcessor* m_dsp;
    
    // Configuration
    SAMVoiceParams m_voiceParams;
    SAMConfig m_config;
    
    // State
    bool m_initialized;
    bool m_isSpeaking;
    float m_progress;
    
    // Synthesis functions
    void generateFormants(const Phoneme& phoneme, float* buffer, size_t samples);
    void generateTransition(const Phoneme& from, const Phoneme& to, 
                          float* buffer, size_t samples);
    void applyEnvelope(float* buffer, size_t samples, uint8_t amplitude);
    
    // Text processing helpers
    void convertWordToPhonemes(const String& word, PhonemeSequence& sequence);
    void applySentenceIntonation(PhonemeSequence& sequence, 
                                size_t startIdx, size_t endIdx);
    
    // Formant synthesis
    float generateFormantSample(float freq, float amp, float bw, float phase);
    
    // Constants
    static constexpr uint32_t SAM_SAMPLE_RATE = 22050;
    static constexpr size_t MAX_PHONEMES = 256;
    static constexpr float PI = 3.14159265359f;
    static constexpr float TWO_PI = 6.28318530718f;
};

#endif // SAM_ENGINE_H