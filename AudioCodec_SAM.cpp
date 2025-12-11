/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM SPEECH SYNTHESIS CODEC - Implementation                                ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#include "AudioCodec_SAM.h"
#include "AudioFilesystem.h"
#include <FS.h>

// Static extension list
const char* AudioCodec_SAM::s_extensions[] = {".txt", nullptr};

// ═══════════════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════

AudioCodec_SAM::AudioCodec_SAM(AudioFilesystem* fs)
    : m_filesystem(fs)
    , m_bufferPosition(0)
    , m_isOpen(false)
    , m_targetSampleRate(22050)
{
    m_audioBuffer.reserve(8192);
}

AudioCodec_SAM::~AudioCodec_SAM() {
    close();
}

// ═══════════════════════════════════════════════════════════════════════════
// AudioCodec Interface - Metadata
// ═══════════════════════════════════════════════════════════════════════════

const char* AudioCodec_SAM::getName() {
    return "SAM Speech Synthesis";
}

const char* AudioCodec_SAM::getVersion() {
    return "2.0 ESP32";
}

const char** AudioCodec_SAM::getExtensions() {
    return s_extensions;
}

CodecCapabilities AudioCodec_SAM::getCapabilities() {
    CodecCapabilities caps;
    caps.canDecode = true;
    caps.canEncode = true;
    caps.canSeek = false;
    caps.supportsStreaming = false;
    caps.requiresFullLoad = true;
    return caps;
}

// ═══════════════════════════════════════════════════════════════════════════
// AudioCodec Interface - File Operations
// ═══════════════════════════════════════════════════════════════════════════

bool AudioCodec_SAM::probe(const char* filename) {
    if (!filename) return false;
    
    String fn(filename);
    fn.toLowerCase();
    
    // SAM accepts text files or plain text input
    if (fn.endsWith(".txt")) return true;
    
    // If no extension, assume it's text to synthesize
    if (fn.indexOf('.') == -1) return true;
    
    return false;
}

bool AudioCodec_SAM::open(const char* filename) {
    if (!filename) return false;
    
    close();
    
    String text(filename);
    
    // If it's a file path with extension, try to load from filesystem
    if (text.indexOf('.') != -1 && m_filesystem) {
        String path = text;
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        
        File file = m_filesystem->open(path);
        if (file) {
            text = "";
            while (file.available()) {
                text += (char)file.read();
            }
            file.close();
        } else {
            Serial.printf("[SAM] Warning: Could not open file '%s', using as text\n", path.c_str());
        }
    }
    
    // Store text and mark as open
    m_currentText = text;
    m_isOpen = true;
    m_bufferPosition = 0;
    
    // Generate audio
    generateAudio();
    
    return m_audioBuffer.size() > 0;
}

void AudioCodec_SAM::close() {
    m_isOpen = false;
    m_currentText = "";
    m_audioBuffer.clear();
    m_bufferPosition = 0;
}

bool AudioCodec_SAM::isOpen() {
    return m_isOpen;
}

// ═══════════════════════════════════════════════════════════════════════════
// AudioCodec Interface - Format & Reading
// ═══════════════════════════════════════════════════════════════════════════

AudioFormat AudioCodec_SAM::getFormat() {
    AudioFormat fmt;
    fmt.sampleRate = m_targetSampleRate;
    fmt.bitsPerSample = 16;
    fmt.channels = 1;
    fmt.bitrate = m_targetSampleRate * 16;
    fmt.duration = m_audioBuffer.size() / m_targetSampleRate;
    return fmt;
}

size_t AudioCodec_SAM::read(void* buffer, size_t size) {
    if (!m_isOpen || !buffer) return 0;
    
    size_t bytesAvailable = (m_audioBuffer.size() - m_bufferPosition) * sizeof(int16_t);
    size_t bytesToRead = (size < bytesAvailable) ? size : bytesAvailable;
    
    if (bytesToRead > 0) {
        memcpy(buffer, 
               &m_audioBuffer[m_bufferPosition], 
               bytesToRead);
        m_bufferPosition += bytesToRead / sizeof(int16_t);
    }
    
    return bytesToRead;
}

bool AudioCodec_SAM::seek(uint32_t position) {
    // SAM doesn't support seeking in generated audio
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// AudioCodec Interface - Sample Rate
// ═══════════════════════════════════════════════════════════════════════════

void AudioCodec_SAM::setTargetSampleRate(uint32_t rate) {
    m_targetSampleRate = rate;
}

uint32_t AudioCodec_SAM::getTargetSampleRate() {
    return m_targetSampleRate;
}

// ═══════════════════════════════════════════════════════════════════════════
// SAM-Specific Functions
// ═══════════════════════════════════════════════════════════════════════════

bool AudioCodec_SAM::synthesizeText(const String& text) {
    m_currentText = text;
    m_bufferPosition = 0;
    generateAudio();
    return m_audioBuffer.size() > 0;
}

bool AudioCodec_SAM::applyPreset(SAMVoicePreset preset) {
    m_samEngine.applyPreset(preset);
    // Regenerate if we have text
    if (!m_currentText.isEmpty()) {
        generateAudio();
    }
    return true;
}

bool AudioCodec_SAM::setVoiceParams(const SAMVoiceParams& params) {
    m_samEngine.setVoiceParams(params);
    // Regenerate if we have text
    if (!m_currentText.isEmpty()) {
        generateAudio();
    }
    return true;
}

void AudioCodec_SAM::getVoiceParams(SAMVoiceParams& params) const {
    params = m_samEngine.getVoiceParams();
}

// ═══════════════════════════════════════════════════════════════════════════
// Private: Audio Generation
// ═══════════════════════════════════════════════════════════════════════════

void AudioCodec_SAM::generateAudio() {
    m_audioBuffer.clear();
    m_bufferPosition = 0;
    
    if (m_currentText.isEmpty()) return;
    
    Serial.printf("[SAM] Synthesizing: '%s'\n", m_currentText.c_str());
    
    // Convert text to phonemes
    PhonemeSequence sequence = m_samEngine.textToPhonemes(m_currentText);
    
    // Apply prosody
    m_samEngine.applyProsody(sequence);
    
    // Synthesize
    std::vector<float> floatBuffer;
    size_t samples = m_samEngine.synthesize(sequence, floatBuffer);
    
    if (samples == 0) {
        Serial.println("[SAM] Error: No audio generated");
        return;
    }
    
    // Convert to int16_t
    m_audioBuffer.resize(samples);
    for (size_t i = 0; i < samples; i++) {
        float sample = floatBuffer[i];
        sample = std::max(-1.0f, std::min(1.0f, sample));
        m_audioBuffer[i] = static_cast<int16_t>(sample * 32767.0f);
    }
    
    Serial.printf("[SAM] Generated %u samples (%.2f seconds)\n", 
                  samples, (float)samples / m_targetSampleRate);
}