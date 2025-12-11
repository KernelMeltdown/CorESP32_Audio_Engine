// AudioCodec_SAM.cpp - SAM Speech Synthesis Implementation

#include "AudioCodec_SAM.h"

// Static extensions array
const char* AudioCodec_SAM::s_extensions[] = {"sam", "txt", "speech", nullptr};

AudioCodec_SAM::AudioCodec_SAM(AudioFilesystem* fs)
    : m_filesystem(fs)
    , m_readPosition(0)
    , m_isOpen(false)
    , m_isPlaying(false)
    , m_targetSampleRate(22050)
{
    initFormat();
    
    // SAMEngine initialisieren (ohne AudioEngine)
    if (!m_samEngine.begin(nullptr)) {
        Serial.println(F("[SAM] Failed to initialize engine"));
    } else {
        // Lade Konfiguration falls vorhanden
        m_samEngine.loadConfig("/sam_config.json");
        
        // Setze Standard-Preset
        m_samEngine.applyPreset(SAMVoicePreset::NATURAL);
        
        Serial.println(F("[SAM] Engine initialized"));
    }
}

AudioCodec_SAM::~AudioCodec_SAM() {
    close();
    m_samEngine.end();
}

const char** AudioCodec_SAM::getExtensions() {
    return s_extensions;
}

CodecCapabilities AudioCodec_SAM::getCapabilities() {
    CodecCapabilities caps;
    caps.canDecode = true;
    caps.canEncode = false;  // Wir "dekodieren" Text zu Audio
    caps.canStream = true;
    caps.canResample = false; // Könnte mit AudioResampler gemacht werden
    caps.maxSampleRate = 22050;
    caps.maxChannels = 1;
    caps.maxBitDepth = 16;
    caps.ramUsage = 50000;  // ~50KB für Buffer
    caps.cpuUsage = 0.15f;  // ~15% CPU
    return caps;
}

bool AudioCodec_SAM::probe(const char* filename) {
    if (!filename) return false;
    
    String fn = String(filename);
    fn.toLowerCase();
    
    // SAM kann alles "dekodieren" was wie Text aussieht
    // Prüfe auf bekannte Extensions
    if (fn.endsWith(".sam") || fn.endsWith(".txt") || fn.endsWith(".speech")) {
        return true;
    }
    
    // Oder wenn es kein Punkt hat (direkter Text)
    if (fn.indexOf('.') == -1) {
        return true;
    }
    
    return false;
}

bool AudioCodec_SAM::open(const char* path) {
    if (!path || strlen(path) == 0) {
        Serial.println(F("[SAM] Error: Empty input"));
        return false;
    }
    
    // Bei SAM ist "path" entweder:
    // 1. Direkter Text (kein Dateipfad)
    // 2. Pfad zu Textdatei
    
    String text(path);
    
    // Prüfe ob es ein Dateipfad ist
    if (text.indexOf('.') != -1 && m_filesystem) {
        // Versuche Datei zu lesen
        File file = m_filesystem->open(path);
        if (file) {
            text = "";
            while (file.available()) {
                text += (char)file.read();
            }
            file.close();
            Serial.printf("[SAM] Loaded text from file: %s\n", path);
        }
    }
    
    return synthesizeText(text);
}

void AudioCodec_SAM::close() {
    clearBuffer();
    m_isOpen = false;
    m_isPlaying = false;
    m_readPosition = 0;
}

size_t AudioCodec_SAM::read(int16_t* buffer, size_t samples) {
    if (!m_isOpen || !buffer || samples == 0) {
        return 0;
    }
    
    // Berechne wie viele Samples noch verfügbar sind
    size_t available = m_audioBuffer.size() - m_readPosition;
    if (available == 0) {
        m_isPlaying = false;
        return 0;
    }
    
    // Kopiere Samples
    size_t toRead = (samples < available) ? samples : available;
    memcpy(buffer, &m_audioBuffer[m_readPosition], toRead * sizeof(int16_t));
    m_readPosition += toRead;
    
    m_isPlaying = (m_readPosition < m_audioBuffer.size());
    
    return toRead;
}

bool AudioCodec_SAM::seek(uint32_t position) {
    if (!m_isOpen) {
        return false;
    }
    
    // Position in Samples
    if (position >= m_audioBuffer.size()) {
        m_readPosition = m_audioBuffer.size();
        m_isPlaying = false;
        return false;
    }
    
    m_readPosition = position;
    m_isPlaying = true;
    return true;
}

AudioFormat AudioCodec_SAM::getFormat() {
    return m_format;
}

// ============================================================================
// SAM-spezifische Funktionen
// ============================================================================

bool AudioCodec_SAM::synthesizeText(const String& text) {
    if (text.isEmpty()) {
        Serial.println(F("[SAM] Error: Empty text"));
        return false;
    }
    
    Serial.printf("[SAM] Synthesizing: %s\n", text.c_str());
    
    // Generiere Audio in Buffer
    size_t requiredSamples = m_samEngine.generateBuffer(
        text, nullptr, 0, m_format.sampleRate
    );
    
    if (requiredSamples == 0) {
        Serial.println(F("[SAM] Error: Synthesis failed"));
        return false;
    }
    
    // Allokiere Buffer
    if (!allocateBuffer(requiredSamples)) {
        Serial.println(F("[SAM] Error: Buffer allocation failed"));
        return false;
    }
    
    // Generiere tatsächliches Audio
    size_t actualSamples = m_samEngine.generateBuffer(
        text, 
        m_audioBuffer.data(), 
        requiredSamples, 
        m_format.sampleRate
    );
    
    if (actualSamples == 0) {
        Serial.println(F("[SAM] Error: Audio generation failed"));
        clearBuffer();
        return false;
    }
    
    // Passe Buffer-Größe an falls nötig
    if (actualSamples < requiredSamples) {
        m_audioBuffer.resize(actualSamples);
    }
    
    m_readPosition = 0;
    m_isOpen = true;
    m_isPlaying = true;
    
    Serial.printf("[SAM] Generated %u samples (%.2f sec)\n", 
                 actualSamples, (float)actualSamples / m_format.sampleRate);
    
    return true;
}

void AudioCodec_SAM::setVoicePreset(SAMVoicePreset preset) {
    m_samEngine.applyPreset(preset);
}

void AudioCodec_SAM::setVoiceParams(const SAMVoiceParams& params) {
    m_samEngine.setVoiceParams(params);
}

SAMVoiceParams AudioCodec_SAM::getVoiceParams() const {
    return m_samEngine.getVoiceParams();
}

void AudioCodec_SAM::enableDebug(bool enable) {
    m_samEngine.setDebugMode(enable);
}

uint32_t AudioCodec_SAM::getDuration() const {
    if (!m_isOpen || m_audioBuffer.empty()) {
        return 0;
    }
    
    // Duration in Millisekunden
    return (m_audioBuffer.size() * 1000) / m_format.sampleRate;
}

uint32_t AudioCodec_SAM::getPosition() const {
    if (!m_isOpen) {
        return 0;
    }
    
    // Position in Millisekunden
    return (m_readPosition * 1000) / m_format.sampleRate;
}

// ============================================================================
// Helper Functions
// ============================================================================

void AudioCodec_SAM::clearBuffer() {
    m_audioBuffer.clear();
    m_audioBuffer.shrink_to_fit();
}

bool AudioCodec_SAM::allocateBuffer(size_t samples) {
    try {
        clearBuffer();
        m_audioBuffer.resize(samples);
        return true;
    } catch (...) {
        return false;
    }
}

void AudioCodec_SAM::initFormat() {
    m_format.sampleRate = 22050;
    m_format.channels = 1;
    m_format.bitDepth = 16;
}