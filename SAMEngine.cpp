/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM SPEECH SYNTHESIS ENGINE - Implementation                                ║
 ║  Modern ESP32 Implementation                                                 ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#include "SAMEngine.h"
#include "SAMPhonemes.h"
#include "SAMDSPProcessor.h"
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════

SAMEngine::SAMEngine()
    : m_audioEngine(nullptr)
    , m_dsp(nullptr)
    , m_initialized(false)
    , m_isSpeaking(false)
    , m_progress(0.0f)
{
}

SAMEngine::~SAMEngine() {
    end();
}

// ═══════════════════════════════════════════════════════════════════════════
// Initialization
// ═══════════════════════════════════════════════════════════════════════════

bool SAMEngine::begin(AudioEngine* engine) {
    if (m_initialized) {
        return true;
    }
    
    m_audioEngine = engine;
    
    // Initialize DSP processor
    m_dsp = new SAMDSPProcessor();
    if (!m_dsp) {
        Serial.println("[SAM] ERROR: Failed to create DSP processor");
        return false;
    }
    
    // Initialize text-to-phoneme rules
    SAMTextRules::initializeRules();
    
    // Apply default voice preset
    applyPreset(SAMVoicePreset::NATURAL);
    
    m_initialized = true;
    Serial.println("[SAM] Engine initialized successfully");
    
    return true;
}

void SAMEngine::end() {
    if (!m_initialized) return;
    
    if (m_dsp) {
        delete m_dsp;
        m_dsp = nullptr;
    }
    
    m_initialized = false;
    Serial.println("[SAM] Engine stopped");
}

// ═══════════════════════════════════════════════════════════════════════════
// Configuration
// ═══════════════════════════════════════════════════════════════════════════

bool SAMEngine::loadConfig(const char* jsonPath) {
    // TODO: Implement JSON loading from SPIFFS
    Serial.printf("[SAM] Config loading from '%s' not yet implemented\n", jsonPath);
    return false;
}

bool SAMEngine::saveConfig(const char* jsonPath) {
    // TODO: Implement JSON saving to SPIFFS
    Serial.printf("[SAM] Config saving to '%s' not yet implemented\n", jsonPath);
    return false;
}

void SAMEngine::setVoiceParams(const SAMVoiceParams& params) {
    m_voiceParams = params;
    Serial.println("[SAM] Voice parameters updated");
}

void SAMEngine::setConfig(const SAMConfig& config) {
    m_config = config;
    Serial.println("[SAM] Configuration updated");
}

// ═══════════════════════════════════════════════════════════════════════════
// Voice Presets
// ═══════════════════════════════════════════════════════════════════════════

void SAMEngine::applyPreset(SAMVoicePreset preset) {
    switch (preset) {
        case SAMVoicePreset::NATURAL:
            m_voiceParams.speed = 72;
            m_voiceParams.pitch = 64;
            m_voiceParams.throat = 128;
            m_voiceParams.mouth = 128;
            m_voiceParams.stress = 0;
            break;
            
        case SAMVoicePreset::CLEAR:
            m_voiceParams.speed = 75;
            m_voiceParams.pitch = 72;
            m_voiceParams.throat = 120;
            m_voiceParams.mouth = 140;
            m_voiceParams.stress = 10;
            break;
            
        case SAMVoicePreset::WARM:
            m_voiceParams.speed = 68;
            m_voiceParams.pitch = 58;
            m_voiceParams.throat = 140;
            m_voiceParams.mouth = 120;
            m_voiceParams.stress = 5;
            break;
            
        case SAMVoicePreset::ROBOT:
            m_voiceParams.speed = 80;
            m_voiceParams.pitch = 64;
            m_voiceParams.throat = 110;
            m_voiceParams.mouth = 110;
            m_voiceParams.stress = 0;
            break;
            
        case SAMVoicePreset::CHILD:
            m_voiceParams.speed = 85;
            m_voiceParams.pitch = 100;
            m_voiceParams.throat = 100;
            m_voiceParams.mouth = 145;
            m_voiceParams.stress = 15;
            break;
            
        case SAMVoicePreset::DEEP:
            m_voiceParams.speed = 65;
            m_voiceParams.pitch = 40;
            m_voiceParams.throat = 150;
            m_voiceParams.mouth = 110;
            m_voiceParams.stress = 8;
            break;
    }
    
    Serial.printf("[SAM] Applied preset (speed=%d, pitch=%d)\n", 
                  m_voiceParams.speed, m_voiceParams.pitch);
}

// ═══════════════════════════════════════════════════════════════════════════
// Main Synthesis Functions
// ═══════════════════════════════════════════════════════════════════════════

bool SAMEngine::speak(const String& text, bool async) {
    if (!m_initialized) {
        Serial.println("[SAM] ERROR: Engine not initialized");
        return false;
    }
    
    if (text.isEmpty()) {
        Serial.println("[SAM] ERROR: Empty text");
        return false;
    }
    
    Serial.printf("[SAM] Speaking: '%s'\n", text.c_str());
    
    // Convert text to phonemes
    PhonemeSequence sequence = textToPhonemes(text);
    
    // Apply prosody
    applyProsody(sequence);
    
    // Synthesize audio
    std::vector<float> audioBuffer;
    size_t samples = synthesize(sequence, audioBuffer);
    
    if (samples == 0) {
        Serial.println("[SAM] ERROR: Synthesis failed");
        return false;
    }
    
    Serial.printf("[SAM] Generated %u samples (%.2f seconds)\n",
                  samples, (float)samples / SAM_SAMPLE_RATE);
    
    // TODO: Send to audio engine if available
    if (m_audioEngine) {
        // m_audioEngine->playBuffer(audioBuffer.data(), samples, SAM_SAMPLE_RATE);
        Serial.println("[SAM] Audio engine playback not yet integrated");
    }
    
    return true;
}

size_t SAMEngine::synthesize(const PhonemeSequence& sequence, std::vector<float>& output) {
    if (sequence.phonemes.empty()) {
        return 0;
    }
    
    // Estimate output size
    size_t estimatedSamples = (sequence.totalDuration * SAM_SAMPLE_RATE) / 1000;
    output.reserve(estimatedSamples);
    output.clear();
    
    // Synthesize each phoneme
    for (size_t i = 0; i < sequence.phonemes.size(); i++) {
        const Phoneme& phoneme = sequence.phonemes[i];
        
        // Calculate phoneme duration in samples
        size_t phonemeSamples = (phoneme.duration * SAM_SAMPLE_RATE) / 1000;
        
        // Generate phoneme audio
        std::vector<float> phonemeBuffer(phonemeSamples, 0.0f);
        generateFormants(phoneme, phonemeBuffer.data(), phonemeSamples);
        
        // Apply amplitude envelope
        applyEnvelope(phonemeBuffer.data(), phonemeSamples, phoneme.amplitude);
        
        // Append to output
        output.insert(output.end(), phonemeBuffer.begin(), phonemeBuffer.end());
    }
    
    // Apply DSP processing if enabled
    if (m_config.enableDSP && m_dsp && !output.empty()) {
        if (m_config.enableSmoothing) {
            m_dsp->applySmoothing(output.data(), output.size(), m_config.smoothingAmount);
        }
        
        if (m_config.enableInterpolation) {
            m_dsp->applyCubicInterpolation(output.data(), output.size(), m_config.interpolationAmount);
        }
        
        if (m_config.enableFormantBoost) {
            // Boost formants around 1000 Hz
            m_dsp->applyFormantBoost(output.data(), output.size(), 1000.0f, 1.2f);
        }
        
        if (m_config.enableBassBoost) {
            m_dsp->applyBassBoost(output.data(), output.size(), 1.3f);
        }
    }
    
    return output.size();
}

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Generation
// ═══════════════════════════════════════════════════════════════════════════

void SAMEngine::generateFormants(const Phoneme& phoneme, float* buffer, size_t samples) {
    if (!buffer || samples == 0) return;
    
    const FormantSet& formants = phoneme.formants;
    
    // Generate each formant and sum
    for (size_t i = 0; i < samples; i++) {
        float t = (float)i / SAM_SAMPLE_RATE;
        float sample = 0.0f;
        
        // Formant 1
        if (formants.f1_freq > 0) {
            float phase1 = 2.0f * 3.14159265359f * formants.f1_freq * t;
            sample += formants.f1_amp * std::sin(phase1);
        }
        
        // Formant 2
        if (formants.f2_freq > 0) {
            float phase2 = 2.0f * 3.14159265359f * formants.f2_freq * t;
            sample += formants.f2_amp * std::sin(phase2);
        }
        
        // Formant 3
        if (formants.f3_freq > 0) {
            float phase3 = 2.0f * 3.14159265359f * formants.f3_freq * t;
            sample += formants.f3_amp * std::sin(phase3);
        }
        
        // Apply pitch modulation
        float pitchMod = 1.0f + (m_voiceParams.pitch - 64.0f) / 128.0f;
        sample *= pitchMod;
        
        // Voiced/unvoiced
        if (!phoneme.voiced) {
            // Add noise for unvoiced phonemes
            sample += ((rand() % 1000) / 1000.0f - 0.5f) * 0.3f;
        }
        
        buffer[i] = sample;
    }
}

void SAMEngine::generateTransition(const Phoneme& from, const Phoneme& to, 
                                   float* buffer, size_t samples) {
    // TODO: Implement smooth transitions between phonemes
    // For now, just blend linearly
    for (size_t i = 0; i < samples; i++) {
        float t = (float)i / samples;
        // Linear blend would go here
    }
}

void SAMEngine::applyEnvelope(float* buffer, size_t samples, uint8_t amplitude) {
    if (!buffer || samples == 0) return;
    
    float amp = amplitude / 255.0f;
    
    // Simple attack-sustain-release envelope
    size_t attackSamples = samples / 10;
    size_t releaseSamples = samples / 10;
    size_t sustainSamples = samples - attackSamples - releaseSamples;
    
    for (size_t i = 0; i < samples; i++) {
        float envelope = 1.0f;
        
        if (i < attackSamples) {
            // Attack
            envelope = (float)i / attackSamples;
        } else if (i >= attackSamples + sustainSamples) {
            // Release
            size_t releaseIdx = i - (attackSamples + sustainSamples);
            envelope = 1.0f - ((float)releaseIdx / releaseSamples);
        }
        
        buffer[i] *= envelope * amp;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Text Processing
// ═══════════════════════════════════════════════════════════════════════════

PhonemeSequence SAMEngine::textToPhonemes(const String& text) {
    PhonemeSequence sequence;
    sequence.totalDuration = 0;
    
    if (text.isEmpty()) return sequence;
    
    // Use text-to-phoneme rules
    String phonemeString = SAMTextRules::textToPhonemes(text);
    
    // Parse phoneme string and create sequence
    // For now, simple word-by-word conversion
    String word = "";
    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        
        if (isAlpha(c) || isDigit(c)) {
            word += c;
        } else {
            if (word.length() > 0) {
                convertWordToPhonemes(word, sequence);
                word = "";
            }
            
            // Add pause for punctuation
            if (c == '.' || c == '!' || c == '?') {
                Phoneme pause;
                strncpy(pause.symbol, " ", sizeof(pause.symbol));
                pause.duration = 150;
                pause.pitch = m_voiceParams.pitch;
                pause.amplitude = 0;
                pause.voiced = false;
                sequence.phonemes.push_back(pause);
                sequence.totalDuration += pause.duration;
            }
        }
    }
    
    // Process last word
    if (word.length() > 0) {
        convertWordToPhonemes(word, sequence);
    }
    
    Serial.printf("[SAM] Created phoneme sequence: %u phonemes, %u ms\n",
                  sequence.phonemes.size(), sequence.totalDuration);
    
    return sequence;
}

void SAMEngine::convertWordToPhonemes(const String& word, PhonemeSequence& sequence) {
    // Simple letter-to-phoneme mapping (very basic)
    for (int i = 0; i < word.length(); i++) {
        char c = tolower(word.charAt(i));
        Phoneme ph;
        
        // Basic English letter-to-phoneme
        switch (c) {
            case 'a':
                ph = {{'A','E',0,0}, 100, m_voiceParams.pitch, 200, true, 
                      SAMFormantTables::AE_DATA.formants};
                break;
            case 'e':
                ph = {{'E','H',0,0}, 90, m_voiceParams.pitch, 190, true,
                      SAMFormantTables::EH_DATA.formants};
                break;
            case 'i':
                ph = {{'I','H',0,0}, 80, m_voiceParams.pitch, 180, true,
                      SAMFormantTables::IH_DATA.formants};
                break;
            case 'o':
                ph = {{'A','A',0,0}, 110, m_voiceParams.pitch, 210, true,
                      SAMFormantTables::AA_DATA.formants};
                break;
            case 'u':
                ph = {{'U','H',0,0}, 95, m_voiceParams.pitch, 195, true,
                      SAMFormantTables::UH_DATA.formants};
                break;
            default:
                // Consonants - use simplified mapping
                ph = {{'X',0,0,0}, 70, m_voiceParams.pitch, 150, false,
                      SAMFormantTables::SILENCE_DATA.formants};
                break;
        }
        
        sequence.phonemes.push_back(ph);
        sequence.totalDuration += ph.duration;
    }
    
    // Add short pause between words
    Phoneme pause;
    strncpy(pause.symbol, " ", sizeof(pause.symbol));
    pause.duration = 50;
    pause.pitch = m_voiceParams.pitch;
    pause.amplitude = 0;
    pause.voiced = false;
    sequence.phonemes.push_back(pause);
    sequence.totalDuration += pause.duration;
}

void SAMEngine::applyProsody(PhonemeSequence& sequence) {
    if (sequence.phonemes.empty()) return;
    
    // Apply sentence-level intonation
    size_t sentenceStart = 0;
    bool inSentence = false;
    
    for (size_t i = 0; i < sequence.phonemes.size(); i++) {
        Phoneme& ph = sequence.phonemes[i];
        
        // Detect sentence boundaries
        if (ph.symbol[0] != ' ' && !inSentence) {
            sentenceStart = i;
            inSentence = true;
        } else if (ph.duration > 100 && inSentence) {
            // End of sentence
            applySentenceIntonation(sequence, sentenceStart, i);
            inSentence = false;
        }
    }
    
    // Apply to last sentence if needed
    if (inSentence) {
        applySentenceIntonation(sequence, sentenceStart, sequence.phonemes.size());
    }
}

void SAMEngine::applySentenceIntonation(PhonemeSequence& sequence, 
                                       size_t startIdx, size_t endIdx) {
    if (endIdx <= startIdx) return;
    
    size_t sentenceLength = endIdx - startIdx;
    
    for (size_t i = startIdx; i < endIdx; i++) {
        Phoneme& ph = sequence.phonemes[i];
        
        // Calculate position in sentence (0.0 to 1.0)
        float position = (float)(i - startIdx) / sentenceLength;
        
        // Apply pitch contour (rise then fall)
        float pitchModifier = 1.0f;
        if (position < 0.3f) {
            // Rising
            pitchModifier = 1.0f + position * 0.2f;
        } else if (position > 0.7f) {
            // Falling
            pitchModifier = 1.06f - (position - 0.7f) * 0.3f;
        } else {
            // Sustain
            pitchModifier = 1.06f;
        }
        
        ph.pitch = (uint8_t)(ph.pitch * pitchModifier);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Helper Functions
// ═══════════════════════════════════════════════════════════════════════════

float SAMEngine::generateFormantSample(float freq, float amp, float bw, float phase) {
    // Simple formant oscillator
    return amp * std::sin(2.0f * 3.14159265359f * freq * phase);
}