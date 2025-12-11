// SAMEngine.cpp - Modern SAM Speech Synthesizer Implementation
// ESP32-optimized, no C64 legacy code
// Part 1: Core functions, initialization, and configuration

#include "SAMEngine.h"
#include "SAMPhonemes.h"
#include "SAMDSPProcessor.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

// ============================================================================
// Constructor & Destructor
// ============================================================================

SAMEngine::SAMEngine() 
    : m_audioEngine(nullptr)
    , m_currentPreset(SAMVoicePreset::NATURAL)
    , m_isSpeaking(false)
    , m_debugMode(false)
    , m_synthTask(nullptr)
    , m_textQueue(nullptr)
    , m_mutex(nullptr)
    , m_audioBuffer(nullptr)
    , m_audioBufferSize(DEFAULT_BUFFER_SIZE)
    , m_dspProcessor(nullptr)
{
    // Initialize oscillator state
    m_oscState.phase1 = 0;
    m_oscState.phase2 = 0;
    m_oscState.phase3 = 0;
    m_oscState.lastFreq1 = 500;
    m_oscState.lastFreq2 = 1500;
    m_oscState.lastFreq3 = 2500;
    
    // Initialize stats
    memset(&m_stats, 0, sizeof(Stats));
    
    // Apply default preset
    applyPreset(SAMVoicePreset::NATURAL);
}

SAMEngine::~SAMEngine() {
    end();
}

// ============================================================================
// Initialization & Configuration
// ============================================================================

bool SAMEngine::begin(AudioEngine* audioEngine) {
    if (!audioEngine) {
        Serial.println("[SAM] Error: AudioEngine is null");
        return false;
    }
    
    m_audioEngine = audioEngine;
    
    // Create mutex for thread safety
    m_mutex = xSemaphoreCreateMutex();
    if (!m_mutex) {
        Serial.println("[SAM] Error: Failed to create mutex");
        return false;
    }
    
    // Create text queue
    m_textQueue = xQueueCreate(MAX_TEXT_QUEUE_SIZE, sizeof(char*));
    if (!m_textQueue) {
        Serial.println("[SAM] Error: Failed to create queue");
        vSemaphoreDelete(m_mutex);
        return false;
    }
    
    // Allocate audio buffer (use PSRAM if available)
    if (psramFound()) {
        m_audioBuffer = (float*)ps_malloc(m_audioBufferSize * sizeof(float));
        Serial.println("[SAM] Using PSRAM for audio buffer");
    } else {
        m_audioBuffer = (float*)malloc(m_audioBufferSize * sizeof(float));
        Serial.println("[SAM] Using heap for audio buffer");
    }
    
    if (!m_audioBuffer) {
        Serial.println("[SAM] Error: Failed to allocate audio buffer");
        vQueueDelete(m_textQueue);
        vSemaphoreDelete(m_mutex);
        return false;
    }
    
    // Initialize DSP processor
    m_dspProcessor = new SAMDSPProcessor();
    if (!m_dspProcessor) {
        Serial.println("[SAM] Error: Failed to create DSP processor");
        free(m_audioBuffer);
        vQueueDelete(m_textQueue);
        vSemaphoreDelete(m_mutex);
        return false;
    }
    m_dspProcessor->begin(SAM_SAMPLE_RATE);
    
    // Load configuration
    if (!loadConfig()) {
        Serial.println("[SAM] Warning: Could not load config, using defaults");
    }
    
    // Create synthesis task on Core 1
    BaseType_t result = xTaskCreatePinnedToCore(
        synthTaskFunc,
        "SAM_Synth",
        8192,
        this,
        10,  // High priority for audio
        &m_synthTask,
        1    // Core 1
    );
    
    if (result != pdPASS) {
        Serial.println("[SAM] Error: Failed to create synthesis task");
        delete m_dspProcessor;
        free(m_audioBuffer);
        vQueueDelete(m_textQueue);
        vSemaphoreDelete(m_mutex);
        return false;
    }
    
    Serial.println("[SAM] Engine initialized successfully");
    return true;
}

void SAMEngine::end() {
    // Stop synthesis task
    if (m_synthTask) {
        vTaskDelete(m_synthTask);
        m_synthTask = nullptr;
    }
    
    // Clean up resources
    if (m_textQueue) {
        clearQueue();
        vQueueDelete(m_textQueue);
        m_textQueue = nullptr;
    }
    
    if (m_mutex) {
        vSemaphoreDelete(m_mutex);
        m_mutex = nullptr;
    }
    
    if (m_dspProcessor) {
        delete m_dspProcessor;
        m_dspProcessor = nullptr;
    }
    
    if (m_audioBuffer) {
        free(m_audioBuffer);
        m_audioBuffer = nullptr;
    }
    
    m_audioEngine = nullptr;
}

// ============================================================================
// Configuration Loading/Saving
// ============================================================================

bool SAMEngine::loadConfig(const char* jsonPath) {
    if (!SPIFFS.begin(true)) {
        Serial.println("[SAM] Warning: SPIFFS not available");
        return false;
    }
    
    File file = SPIFFS.open(jsonPath, "r");
    if (!file) {
        Serial.printf("[SAM] Config file not found: %s\n", jsonPath);
        return false;
    }
    
    // Parse JSON
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("[SAM] JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    // Load preset if specified
    JsonObject samEngine = doc["samEngine"];
    if (samEngine.containsKey("currentPreset")) {
        String presetName = samEngine["currentPreset"].as<String>();
        if (presetName == "natural") applyPreset(SAMVoicePreset::NATURAL);
        else if (presetName == "clear") applyPreset(SAMVoicePreset::CLEAR);
        else if (presetName == "warm") applyPreset(SAMVoicePreset::WARM);
        else if (presetName == "robot") applyPreset(SAMVoicePreset::ROBOT);
    }
    
    // Override with custom parameters if present
    if (samEngine.containsKey("customParams")) {
        JsonObject params = samEngine["customParams"];
        m_voiceParams.speed = params["speed"] | m_voiceParams.speed;
        m_voiceParams.pitch = params["pitch"] | m_voiceParams.pitch;
        m_voiceParams.throat = params["throat"] | m_voiceParams.throat;
        m_voiceParams.mouth = params["mouth"] | m_voiceParams.mouth;
        m_voiceParams.smoothing = params["smoothing"] | m_voiceParams.smoothing;
        m_voiceParams.interpolation = params["interpolation"] | m_voiceParams.interpolation;
        m_voiceParams.formantBoost = params["formantBoost"] | m_voiceParams.formantBoost;
        m_voiceParams.bassBoost = params["bassBoost"] | m_voiceParams.bassBoost;
    }
    
    Serial.println("[SAM] Configuration loaded");
    return true;
}

bool SAMEngine::saveConfig(const char* jsonPath) {
    // Create JSON document
    DynamicJsonDocument doc(4096);
    JsonObject samEngine = doc.createNestedObject("samEngine");
    
    // Save current preset
    const char* presetName = "custom";
    switch (m_currentPreset) {
        case SAMVoicePreset::NATURAL: presetName = "natural"; break;
        case SAMVoicePreset::CLEAR: presetName = "clear"; break;
        case SAMVoicePreset::WARM: presetName = "warm"; break;
        case SAMVoicePreset::ROBOT: presetName = "robot"; break;
        default: break;
    }
    samEngine["currentPreset"] = presetName;
    
    // Save current parameters
    JsonObject params = samEngine.createNestedObject("customParams");
    params["speed"] = m_voiceParams.speed;
    params["pitch"] = m_voiceParams.pitch;
    params["throat"] = m_voiceParams.throat;
    params["mouth"] = m_voiceParams.mouth;
    params["smoothing"] = m_voiceParams.smoothing;
    params["interpolation"] = m_voiceParams.interpolation;
    params["formantBoost"] = m_voiceParams.formantBoost;
    params["bassBoost"] = m_voiceParams.bassBoost;
    
    // Write to file
    File file = SPIFFS.open(jsonPath, "w");
    if (!file) {
        Serial.println("[SAM] Error: Could not open file for writing");
        return false;
    }
    
    serializeJsonPretty(doc, file);
    file.close();
    
    Serial.println("[SAM] Configuration saved");
    return true;
}

// ============================================================================
// Voice Parameter Control
// ============================================================================

void SAMEngine::setVoiceParams(const SAMVoiceParams& params) {
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        m_voiceParams = params;
        m_currentPreset = SAMVoicePreset::CUSTOM;
        xSemaphoreGive(m_mutex);
        
        if (m_debugMode) {
            Serial.println("[SAM] Voice parameters updated");
        }
    }
}

void SAMEngine::applyPreset(SAMVoicePreset preset) {
    m_currentPreset = preset;
    
    switch (preset) {
        case SAMVoicePreset::NATURAL:
            m_voiceParams = SAMPresets::getNatural();
            break;
        case SAMVoicePreset::CLEAR:
            m_voiceParams = SAMPresets::getClear();
            break;
        case SAMVoicePreset::WARM:
            m_voiceParams = SAMPresets::getWarm();
            break;
        case SAMVoicePreset::ROBOT:
            m_voiceParams = SAMPresets::getRobot();
            break;
        default:
            break;
    }
    
    if (m_debugMode) {
        Serial.printf("[SAM] Applied preset: %d\n", (int)preset);
    }
}

// ============================================================================
// Preset Definitions
// ============================================================================

namespace SAMPresets {
    
    SAMVoiceParams getNatural() {
        SAMVoiceParams params;
        params.speed = 68;
        params.pitch = 70;
        params.throat = 135;
        params.mouth = 140;
        params.smoothing = 40;
        params.interpolation = 45;
        params.formantBoost = 20;
        params.bassBoost = 2;
        params.pitchVariance = 0.12f;
        params.speedVariance = 0.06f;
        params.enableProsody = true;
        return params;
    }
    
    SAMVoiceParams getClear() {
        SAMVoiceParams params;
        params.speed = 75;
        params.pitch = 64;
        params.throat = 128;
        params.mouth = 128;
        params.smoothing = 25;
        params.interpolation = 30;
        params.formantBoost = 10;
        params.bassBoost = 0;
        params.pitchVariance = 0.08f;
        params.speedVariance = 0.04f;
        params.enableProsody = true;
        return params;
    }
    
    SAMVoiceParams getWarm() {
        SAMVoiceParams params;
        params.speed = 65;
        params.pitch = 60;
        params.throat = 145;
        params.mouth = 150;
        params.smoothing = 45;
        params.interpolation = 50;
        params.formantBoost = 25;
        params.bassBoost = 4;
        params.pitchVariance = 0.15f;
        params.speedVariance = 0.08f;
        params.enableProsody = true;
        return params;
    }
    
    SAMVoiceParams getRobot() {
        SAMVoiceParams params;
        params.speed = 90;
        params.pitch = 50;
        params.throat = 110;
        params.mouth = 110;
        params.smoothing = 10;
        params.interpolation = 15;
        params.formantBoost = 5;
        params.bassBoost = -2;
        params.pitchVariance = 0.02f;
        params.speedVariance = 0.01f;
        params.enableProsody = false;
        return params;
    }
}

// ============================================================================
// Formant Interpolation
// ============================================================================

FormantSet FormantSet::interpolate(const FormantSet& target, float t) const {
    FormantSet result;
    
    // Clamp t to [0, 1]
    t = constrain(t, 0.0f, 1.0f);
    
    // Linear interpolation for all parameters
    result.f1 = f1 + t * (target.f1 - f1);
    result.f2 = f2 + t * (target.f2 - f2);
    result.f3 = f3 + t * (target.f3 - f3);
    
    result.a1 = a1 + t * (target.a1 - a1);
    result.a2 = a2 + t * (target.a2 - a2);
    result.a3 = a3 + t * (target.a3 - a3);
    
    result.bw1 = bw1 + t * (target.bw1 - bw1);
    result.bw2 = bw2 + t * (target.bw2 - bw2);
    result.bw3 = bw3 + t * (target.bw3 - bw3);
    
    return result;
}

// TO BE CONTINUED IN PART 2...
// Next parts will include:
// - Speech synthesis functions
// - Text-to-phoneme conversion
// - Multi-core synthesis task
// - Audio generation
// - DSP processing integration

// SAMEngine.cpp - Part 2: Speech Synthesis Functions
// Continues from Part 1

// ============================================================================
// Speech Synthesis - Main API
// ============================================================================

bool SAMEngine::speak(const String& text, bool async) {
    if (text.isEmpty()) {
        Serial.println("[SAM] Error: Empty text");
        return false;
    }
    
    if (async) {
        // Queue text for async synthesis
        char* textCopy = strdup(text.c_str());
        if (!textCopy) {
            Serial.println("[SAM] Error: Out of memory");
            return false;
        }
        
        if (xQueueSend(m_textQueue, &textCopy, 0) != pdTRUE) {
            Serial.println("[SAM] Error: Queue full");
            free(textCopy);
            return false;
        }
        
        return true;
    } else {
        // Synchronous synthesis
        size_t samples = generateBuffer(text, nullptr, 0, DEFAULT_SAMPLE_RATE);
        if (samples == 0) {
            Serial.println("[SAM] Error: Synthesis failed");
            return false;
        }
        
        // Allocate buffer
        int16_t* buffer = (int16_t*)malloc(samples * sizeof(int16_t));
        if (!buffer) {
            Serial.println("[SAM] Error: Out of memory for audio buffer");
            return false;
        }
        
        // Generate audio
        samples = generateBuffer(text, buffer, samples, SAM_SAMPLE_RATE);
        
        // Play through audio engine
        if (m_audioEngine) {
            // TODO: Integrate with your AudioEngine's playback system
            // m_audioEngine->playBuffer(buffer, samples);
        }
        
        free(buffer);
        return samples > 0;
    }
}

bool SAMEngine::speakPhonemes(const String& phonemeString, bool async) {
    // TODO: Implement direct phoneme input
    // For now, treat as regular text
    return speak(phonemeString, async);
}

void SAMEngine::stop() {
    m_isSpeaking = false;
    clearQueue();
    
    if (m_onSpeechComplete) {
        m_onSpeechComplete();
    }
}

void SAMEngine::clearQueue() {
    char* text;
    while (xQueueReceive(m_textQueue, &text, 0) == pdTRUE) {
        free(text);
    }
}

// ============================================================================
// Multi-Core Synthesis Task
// ============================================================================

void SAMEngine::synthTaskFunc(void* parameter) {
    SAMEngine* engine = static_cast<SAMEngine*>(parameter);
    engine->synthTask();
}

void SAMEngine::synthTask() {
    char* text;
    
    Serial.println("[SAM] Synthesis task started on Core 1");
    
    while (true) {
        // Wait for text to synthesize
        if (xQueueReceive(m_textQueue, &text, portMAX_DELAY) == pdTRUE) {
            m_isSpeaking = true;
            
            if (m_onSpeechStart) {
                m_onSpeechStart();
            }
            
            uint32_t startTime = millis();
            
            // Generate audio buffer
            size_t samples = generateBuffer(text, nullptr, 0, SAM_SAMPLE_RATE);
            if (samples > 0) {
                int16_t* buffer = (int16_t*)malloc(samples * sizeof(int16_t));
                if (buffer) {
                    samples = generateBuffer(text, buffer, samples, SAM_SAMPLE_RATE);
                    
                    // Play through audio engine
                    if (m_audioEngine && samples > 0) {
                        // TODO: Integrate with AudioEngine
                        // m_audioEngine->playBuffer(buffer, samples);
                    }
                    
                    free(buffer);
                }
            }
            
            uint32_t duration = millis() - startTime;
            m_stats.totalSynthesized++;
            m_stats.totalDuration_ms += duration;
            
            free(text);
            
            m_isSpeaking = false;
            
            if (m_onSpeechComplete) {
                m_onSpeechComplete();
            }
            
            if (m_debugMode) {
                Serial.printf("[SAM] Synthesis completed in %u ms\n", duration);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ============================================================================
// Audio Buffer Generation
// ============================================================================

size_t SAMEngine::generateBuffer(const String& text, int16_t* buffer, 
                                 size_t maxSamples, uint32_t sampleRate) {
    // Convert text to phonemes
    PhonemeSequence sequence = textToPhonemes(text);
    
    if (sequence.phonemes.empty()) {
        Serial.println("[SAM] Error: No phonemes generated");
        return 0;
    }
    
    // Apply prosody if enabled
    if (m_voiceParams.enableProsody) {
        applyProsody(sequence);
    }
    
    // Calculate durations
    calculateDurations(sequence);
    
    // Calculate stress
    calculateStress(sequence);
    
    // Estimate required samples
    float totalDuration_s = sequence.totalDuration_ms / 1000.0f;
    size_t requiredSamples = (size_t)(totalDuration_s * sampleRate) + 1024; // Add padding
    
    // If no buffer provided, return required size
    if (!buffer) {
        return requiredSamples;
    }
    
    // Limit to max samples
    if (maxSamples < requiredSamples) {
        Serial.printf("[SAM] Warning: Buffer too small (%u < %u)\n", 
                     maxSamples, requiredSamples);
        requiredSamples = maxSamples;
    }
    
    // Allocate float buffer for synthesis
    float* floatBuffer = (float*)malloc(requiredSamples * sizeof(float));
    if (!floatBuffer) {
        Serial.println("[SAM] Error: Out of memory for float buffer");
        return 0;
    }
    
    // Synthesize phonemes to float buffer
    size_t actualSamples = synthesizePhonemes(sequence, floatBuffer, requiredSamples);
    
    // Apply DSP processing
    if (m_dspProcessor) {
        m_dspProcessor->processBuffer(
            floatBuffer, actualSamples,
            m_voiceParams.smoothing / 100.0f,
            m_voiceParams.interpolation / 100.0f,
            m_voiceParams.formantBoost / 100.0f,
            m_voiceParams.bassBoost
        );
    }
    
    // Convert to int16
    for (size_t i = 0; i < actualSamples; i++) {
        float sample = floatBuffer[i];
        sample = constrain(sample, -1.0f, 1.0f);
        buffer[i] = (int16_t)(sample * 32767.0f);
    }
    
    free(floatBuffer);
    
    return actualSamples;
}

float* SAMEngine::generateFloatBuffer(const String& text, size_t* outLength, 
                                      uint32_t sampleRate) {
    // Get required size
    size_t samples = generateBuffer(text, nullptr, 0, sampleRate);
    if (samples == 0 || !outLength) {
        return nullptr;
    }
    
    // Allocate buffers
    int16_t* int16Buffer = (int16_t*)malloc(samples * sizeof(int16_t));
    float* floatBuffer = (float*)malloc(samples * sizeof(float));
    
    if (!int16Buffer || !floatBuffer) {
        free(int16Buffer);
        free(floatBuffer);
        return nullptr;
    }
    
    // Generate
    samples = generateBuffer(text, int16Buffer, samples, sampleRate);
    
    // Convert to float
    for (size_t i = 0; i < samples; i++) {
        floatBuffer[i] = int16Buffer[i] / 32768.0f;
    }
    
    free(int16Buffer);
    
    *outLength = samples;
    return floatBuffer;
}

// ============================================================================
// Phoneme Synthesis
// ============================================================================

size_t SAMEngine::synthesizePhonemes(const PhonemeSequence& sequence, 
                                     float* buffer, size_t maxSamples) {
    size_t sampleIndex = 0;
    float sampleRate = SAM_SAMPLE_RATE;
    float deltaTime = 1.0f / sampleRate;
    
    // Process each phoneme
    for (size_t i = 0; i < sequence.phonemes.size(); i++) {
        const Phoneme& phoneme = sequence.phonemes[i];
        const Phoneme& nextPhoneme = (i + 1 < sequence.phonemes.size()) 
                                     ? sequence.phonemes[i + 1] 
                                     : phoneme;
        
        // Callback for phoneme change
        if (m_onPhonemeChange) {
            m_onPhonemeChange(phoneme);
        }
        
        // Calculate number of samples for this phoneme
        float duration_s = phoneme.duration_ms / 1000.0f;
        
        // Apply speed modulation if set
        if (m_speedModulator) {
            float speedFactor = m_speedModulator(sampleIndex / sampleRate);
            duration_s *= speedFactor;
        }
        
        size_t phonemeSamples = (size_t)(duration_s * sampleRate);
        
        // Generate formant frame for each sample
        for (size_t j = 0; j < phonemeSamples && sampleIndex < maxSamples; j++) {
            float t = (float)j / phonemeSamples;  // 0.0 to 1.0 through phoneme
            
            FormantSet formants;
            generateFormantFrame(phoneme, t, formants);
            
            // Interpolate towards next phoneme in last 30% of duration
            if (t > 0.7f && i + 1 < sequence.phonemes.size()) {
                float interpT = (t - 0.7f) / 0.3f;  // 0.0 to 1.0 in last 30%
                FormantSet nextFormants;
                generateFormantFrame(nextPhoneme, 0.0f, nextFormants);
                formants = formants.interpolate(nextFormants, interpT);
            }
            
            // Render frame to output
            renderFrame(formants, &buffer[sampleIndex], 1);
            
            // Update oscillator phases
            updateOscillatorPhases(deltaTime);
            
            sampleIndex++;
        }
    }
    
    return sampleIndex;
}

// ============================================================================
// Formant Generation
// ============================================================================

void SAMEngine::generateFormantFrame(const Phoneme& phoneme, float t, 
                                     FormantSet& output) {
    // Get base formant data from phoneme table
    const PhonemeFormantData& data = SAMFormantTables::PHONEME_TABLE[phoneme.index];
    
    output = data.formants;
    
    // Apply throat/mouth parameters to modify formants
    float throatScale = m_voiceParams.throat / 128.0f;
    float mouthScale = m_voiceParams.mouth / 128.0f;
    
    output.f1 *= throatScale;
    output.f2 *= (throatScale + mouthScale) / 2.0f;
    output.f3 *= mouthScale;
    
    output.a1 *= mouthScale;
    output.a2 *= mouthScale;
    output.a3 *= mouthScale;
    
    // Apply stress
    float stressScale = 0.5f + phoneme.stress * 0.5f;  // 0.5 to 1.0
    output.a1 *= stressScale;
    output.a2 *= stressScale;
    output.a3 *= stressScale;
    
    // Apply pitch modulation
    float pitchFactor = m_voiceParams.pitch / 64.0f;  // 64 is baseline
    if (m_pitchModulator) {
        pitchFactor *= m_pitchModulator(t);
    }
    
    // All formants shift with pitch (simplified model)
    output.f1 *= pitchFactor;
    output.f2 *= pitchFactor;
    output.f3 *= pitchFactor;
}

void SAMEngine::renderFrame(const FormantSet& formants, float* output, 
                           size_t frameSize) {
    // Generate formant samples using oscillators
    for (size_t i = 0; i < frameSize; i++) {
        float sample = 0;
        
        // Add each formant
        sample += generateFormantSample(formants.f1, formants.a1, formants.bw1, m_oscState.phase1);
        sample += generateFormantSample(formants.f2, formants.a2, formants.bw2, m_oscState.phase2);
        sample += generateFormantSample(formants.f3, formants.a3, formants.bw3, m_oscState.phase3);
        
        output[i] = sample;
    }
}

float SAMEngine::generateFormantSample(float frequency, float amplitude, 
                                       float bandwidth, float phase) {
    // Simple formant synthesis using sinusoid
    // In a more advanced implementation, use resonant filters
    float sample = sinf(phase * 2.0f * PI) * amplitude;
    
    // Apply simple damping based on bandwidth
    float damping = 1.0f - (bandwidth / 1000.0f);  // Rough approximation
    sample *= damping;
    
    return sample;
}

void SAMEngine::updateOscillatorPhases(float deltaTime) {
    // Update phases based on last set frequencies
    m_oscState.phase1 += m_oscState.lastFreq1 * deltaTime;
    m_oscState.phase2 += m_oscState.lastFreq2 * deltaTime;
    m_oscState.phase3 += m_oscState.lastFreq3 * deltaTime;
    
    // Wrap phases to [0, 1]
    while (m_oscState.phase1 >= 1.0f) m_oscState.phase1 -= 1.0f;
    while (m_oscState.phase2 >= 1.0f) m_oscState.phase2 -= 1.0f;
    while (m_oscState.phase3 >= 1.0f) m_oscState.phase3 -= 1.0f;
}

// TO BE CONTINUED IN PART 3...
// Next: Text-to-phoneme conversion, prosody, etc.

// SAMEngine.cpp - Part 3: Text-to-Phoneme and Prosody Processing

// ============================================================================
// Text-to-Phoneme Conversion (Simplified Implementation)
// ============================================================================

PhonemeSequence SAMEngine::textToPhonemes(const String& text) {
    PhonemeSequence sequence;
    
    // Normalize text
    String normalized = text;
    normalized.toUpperCase();
    normalized.trim();
    
    // Basic word tokenization
    int wordStart = 0;
    bool inWord = false;
    
    for (int i = 0; i <= normalized.length(); i++) {
        char c = (i < normalized.length()) ? normalized[i] : ' ';
        
        bool isLetter = (c >= 'A' && c <= 'Z');
        
        if (isLetter && !inWord) {
            wordStart = i;
            inWord = true;
        } else if (!isLetter && inWord) {
            // Process word
            String word = normalized.substring(wordStart, i);
            convertWordToPhonemes(word, sequence);
            
            // Add pause for punctuation
            if (c == '.' || c == '!' || c == '?') {
                Phoneme pause;
                pause.type = PhonemeType::SILENCE;
                pause.index = SAMPhonemeIndex::PAUSE_LONG;
                pause.duration_ms = 300;
                pause.wordBoundary = true;
                sequence.phonemes.push_back(pause);
            } else if (c == ',') {
                Phoneme pause;
                pause.type = PhonemeType::SILENCE;
                pause.index = SAMPhonemeIndex::PAUSE_SHORT;
                pause.duration_ms = 150;
                pause.wordBoundary = true;
                sequence.phonemes.push_back(pause);
            }
            
            inWord = false;
        }
    }
    
    return sequence;
}

void SAMEngine::convertWordToPhonemes(const String& word, PhonemeSequence& sequence) {
    // This is a simplified implementation
    // In a real system, you would use comprehensive pronunciation rules
    // and a dictionary for exceptions
    
    // Mark word boundary
    if (!sequence.phonemes.empty()) {
        sequence.phonemes.back().wordBoundary = true;
    }
    
    // Simple letter-to-phoneme mapping (very basic, just for demonstration)
    for (size_t i = 0; i < word.length(); i++) {
        char c = word[i];
        Phoneme ph;
        ph.stress = 0.5f;  // Default stress
        ph.wordBoundary = false;
        
        // Map letters to phonemes (simplified)
        switch (c) {
            case 'A':
                ph.type = PhonemeType::VOWEL;
                ph.index = SAMPhonemeIndex::AE;
                ph.duration_ms = 120;
                break;
            case 'E':
                ph.type = PhonemeType::VOWEL;
                ph.index = SAMPhonemeIndex::EH;
                ph.duration_ms = 110;
                break;
            case 'I':
                ph.type = PhonemeType::VOWEL;
                ph.index = SAMPhonemeIndex::IH;
                ph.duration_ms = 100;
                break;
            case 'O':
                ph.type = PhonemeType::VOWEL;
                ph.index = SAMPhonemeIndex::AA;
                ph.duration_ms = 130;
                break;
            case 'U':
                ph.type = PhonemeType::VOWEL;
                ph.index = SAMPhonemeIndex::UH;
                ph.duration_ms = 110;
                break;
            case 'B':
                ph.type = PhonemeType::CONSONANT_STOP;
                ph.index = SAMPhonemeIndex::B;
                ph.duration_ms = 80;
                break;
            case 'D':
                ph.type = PhonemeType::CONSONANT_STOP;
                ph.index = SAMPhonemeIndex::D;
                ph.duration_ms = 70;
                break;
            case 'F':
                ph.type = PhonemeType::CONSONANT_FRICATIVE;
                ph.index = SAMPhonemeIndex::F;
                ph.duration_ms = 100;
                break;
            case 'G':
                ph.type = PhonemeType::CONSONANT_STOP;
                ph.index = SAMPhonemeIndex::G;
                ph.duration_ms = 90;
                break;
            case 'H':
                ph.type = PhonemeType::CONSONANT_FRICATIVE;
                ph.index = SAMPhonemeIndex::H;
                ph.duration_ms = 70;
                break;
            case 'K':
                ph.type = PhonemeType::CONSONANT_STOP;
                ph.index = SAMPhonemeIndex::K;
                ph.duration_ms = 90;
                break;
            case 'L':
                ph.type = PhonemeType::CONSONANT_LIQUID;
                ph.index = SAMPhonemeIndex::L;
                ph.duration_ms = 90;
                break;
            case 'M':
                ph.type = PhonemeType::CONSONANT_NASAL;
                ph.index = SAMPhonemeIndex::M;
                ph.duration_ms = 100;
                break;
            case 'N':
                ph.type = PhonemeType::CONSONANT_NASAL;
                ph.index = SAMPhonemeIndex::N;
                ph.duration_ms = 90;
                break;
            case 'P':
                ph.type = PhonemeType::CONSONANT_STOP;
                ph.index = SAMPhonemeIndex::P;
                ph.duration_ms = 80;
                break;
            case 'R':
                ph.type = PhonemeType::CONSONANT_LIQUID;
                ph.index = SAMPhonemeIndex::R;
                ph.duration_ms = 90;
                break;
            case 'S':
                ph.type = PhonemeType::CONSONANT_FRICATIVE;
                ph.index = SAMPhonemeIndex::S;
                ph.duration_ms = 110;
                break;
            case 'T':
                ph.type = PhonemeType::CONSONANT_STOP;
                ph.index = SAMPhonemeIndex::T;
                ph.duration_ms = 70;
                break;
            case 'V':
                ph.type = PhonemeType::CONSONANT_FRICATIVE;
                ph.index = SAMPhonemeIndex::V;
                ph.duration_ms = 90;
                break;
            case 'W':
                ph.type = PhonemeType::CONSONANT_GLIDE;
                ph.index = SAMPhonemeIndex::W;
                ph.duration_ms = 80;
                break;
            case 'Y':
                ph.type = PhonemeType::CONSONANT_GLIDE;
                ph.index = SAMPhonemeIndex::Y;
                ph.duration_ms = 80;
                break;
            case 'Z':
                ph.type = PhonemeType::CONSONANT_FRICATIVE;
                ph.index = SAMPhonemeIndex::Z;
                ph.duration_ms = 100;
                break;
            default:
                continue;  // Skip unknown characters
        }
        
        sequence.phonemes.push_back(ph);
    }
}

// ============================================================================
// Prosody Application
// ============================================================================

void SAMEngine::applyProsody(PhonemeSequence& sequence) {
    if (sequence.phonemes.empty()) return;
    
    // Find sentence boundaries and apply intonation
    bool inSentence = false;
    size_t sentenceStart = 0;
    
    for (size_t i = 0; i < sequence.phonemes.size(); i++) {
        Phoneme& ph = sequence.phonemes[i];
        
        if (ph.type != PhonemeType::SILENCE) {
            if (!inSentence) {
                sentenceStart = i;
                inSentence = true;
            }
        } else if (inSentence && ph.duration_ms > 200) {
            // End of sentence - apply falling intonation
            applySentenceIntonation(sequence, sentenceStart, i);
            inSentence = false;
        }
    }
    
    // Handle last sentence
    if (inSentence) {
        applySentenceIntonation(sequence, sentenceStart, sequence.phonemes.size());
    }
}

void SAMEngine::applySentenceIntonation(PhonemeSequence& sequence, 
                                       size_t start, size_t end) {
    if (end <= start) return;
    
    size_t length = end - start;
    
    // Apply pitch contour: rise in middle, fall at end
    for (size_t i = start; i < end; i++) {
        Phoneme& ph = sequence.phonemes[i];
        
        if (ph.type != PhonemeType::VOWEL && 
            ph.type != PhonemeType::CONSONANT_NASAL &&
            ph.type != PhonemeType::CONSONANT_LIQUID) {
            continue;
        }
        
        float position = (float)(i - start) / length;  // 0.0 to 1.0
        
        // Pitch rises to 0.4, then falls
        float pitchModifier = 1.0f;
        if (position < 0.4f) {
            pitchModifier = 1.0f + position * 0.3f;  // Rise
        } else {
            pitchModifier = 1.12f - (position - 0.4f) * 0.3f;  // Fall
        }
        
        // Store pitch modifier in stress field (will be applied during synthesis)
        ph.stress *= pitchModifier;
    }
}

void SAMEngine::calculateDurations(PhonemeSequence& sequence) {
    float speedFactor = 72.0f / m_voiceParams.speed;  // 72 is baseline
    float totalDuration = 0;
    
    for (Phoneme& ph : sequence.phonemes) {
        // Modify duration based on stress
        float stressMod = 0.85f + ph.stress * 0.3f;  // 0.85 to 1.15
        
        // Modify based on word boundaries
        float boundaryMod = ph.wordBoundary ? 1.2f : 1.0f;
        
        // Apply all modifiers
        ph.duration_ms *= speedFactor * stressMod * boundaryMod;
        
        // Add variance if enabled
        if (m_voiceParams.speedVariance > 0) {
            float variance = (random(-100, 100) / 100.0f) * m_voiceParams.speedVariance;
            ph.duration_ms *= (1.0f + variance);
        }
        
        totalDuration += ph.duration_ms;
    }
    
    sequence.totalDuration_ms = totalDuration;
}

void SAMEngine::calculateStress(PhonemeSequence& sequence) {
    // Simple stress pattern: emphasize first syllable of each word
    bool firstSyllableInWord = true;
    
    for (Phoneme& ph : sequence.phonemes) {
        if (ph.type == PhonemeType::VOWEL) {
            if (firstSyllableInWord) {
                ph.stress *= 1.3f;  // Primary stress
                firstSyllableInWord = false;
            }
        }
        
        if (ph.wordBoundary) {
            firstSyllableInWord = true;
        }
    }
}

// ============================================================================
// Real-time Parameter Modulation
// ============================================================================

void SAMEngine::setPitchModulation(std::function<float(float)> modulator) {
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        m_pitchModulator = modulator;
        xSemaphoreGive(m_mutex);
    }
}

void SAMEngine::setSpeedModulation(std::function<float(float)> modulator) {
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        m_speedModulator = modulator;
        xSemaphoreGive(m_mutex);
    }
}

// ============================================================================
// Debug & Diagnostics
// ============================================================================

void SAMEngine::printPhonemeSequence(const String& text) {
    PhonemeSequence sequence = textToPhonemes(text);
    
    Serial.println("\n=== Phoneme Sequence ===");
    Serial.printf("Text: %s\n", text.c_str());
    Serial.printf("Total phonemes: %d\n", sequence.phonemes.size());
    Serial.printf("Total duration: %.2f s\n\n", sequence.totalDuration_ms / 1000.0f);
    
    for (size_t i = 0; i < sequence.phonemes.size(); i++) {
        const Phoneme& ph = sequence.phonemes[i];
        const char* typeName = "UNKNOWN";
        
        switch (ph.type) {
            case PhonemeType::SILENCE: typeName = "SILENCE"; break;
            case PhonemeType::VOWEL: typeName = "VOWEL"; break;
            case PhonemeType::CONSONANT_STOP: typeName = "STOP"; break;
            case PhonemeType::CONSONANT_FRICATIVE: typeName = "FRICATIVE"; break;
            case PhonemeType::CONSONANT_NASAL: typeName = "NASAL"; break;
            case PhonemeType::CONSONANT_LIQUID: typeName = "LIQUID"; break;
            case PhonemeType::CONSONANT_GLIDE: typeName = "GLIDE"; break;
        }
        
        Serial.printf("%3d: %-12s idx=%3d dur=%4d ms stress=%.2f %s\n",
                     i, typeName, ph.index, ph.duration_ms, ph.stress,
                     ph.wordBoundary ? "[WORD]" : "");
    }
    
    Serial.println("========================\n");
}

void SAMEngine::printFormantData(const Phoneme& phoneme) {
    const PhonemeFormantData& data = SAMFormantTables::PHONEME_TABLE[phoneme.index];
    
    Serial.println("\n=== Formant Data ===");
    Serial.printf("Phoneme: %s (index %d)\n", data.symbol, phoneme.index);
    Serial.printf("F1: %.1f Hz (amp: %.2f, bw: %.1f)\n", 
                 data.formants.f1, data.formants.a1, data.formants.bw1);
    Serial.printf("F2: %.1f Hz (amp: %.2f, bw: %.1f)\n", 
                 data.formants.f2, data.formants.a2, data.formants.bw2);
    Serial.printf("F3: %.1f Hz (amp: %.2f, bw: %.1f)\n", 
                 data.formants.f3, data.formants.a3, data.formants.bw3);
    Serial.printf("Duration: %d ms\n", data.baseDuration_ms);
    Serial.printf("Noise: %.2f, Voicing: %.2f\n", 
                 data.noiseLevel, data.voicing);
    Serial.println("====================\n");
}

void SAMEngine::resetStats() {
    memset(&m_stats, 0, sizeof(Stats));
}

// TO BE CONTINUED IN PART 4...
// Next: DSP Processor implementation