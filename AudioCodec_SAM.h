// AudioCodec_SAM.h - SAM Speech Synthesis als AudioCodec
// Angepasst an dein AudioCodec Interface

#ifndef AUDIO_CODEC_SAM_H
#define AUDIO_CODEC_SAM_H

#include "AudioCodec.h"
#include "SAMEngine.h"
#include <vector>

class AudioCodec_SAM : public AudioCodec {
public:
    AudioCodec_SAM(AudioFilesystem* fs);
    virtual ~AudioCodec_SAM();
    
    // AudioCodec Interface - Info
    const char* getName() override { return "SAM"; }
    const char* getVersion() override { return "2.0-ESP32"; }
    const char** getExtensions() override;
    CodecCapabilities getCapabilities() override;
    
    // AudioCodec Interface - File Operations
    bool probe(const char* filename) override;
    bool open(const char* path) override;
    void close() override;
    bool isOpen() override { return m_isOpen; }
    
    // AudioCodec Interface - Streaming
    size_t read(int16_t* buffer, size_t samples) override;
    bool seek(uint32_t position) override;
    
    // AudioCodec Interface - Format Info
    AudioFormat getFormat() override;
    
    // AudioCodec Interface - Resampling
    void setTargetSampleRate(uint32_t rate) override { m_targetSampleRate = rate; }
    uint32_t getTargetSampleRate() override { return m_targetSampleRate; }
    
    // SAM-spezifische Funktionen
    bool synthesizeText(const String& text);
    
    void setVoicePreset(SAMVoicePreset preset);
    void setVoiceParams(const SAMVoiceParams& params);
    SAMVoiceParams getVoiceParams() const;
    
    void enableDebug(bool enable);
    
    // Getter für Playback-Info
    uint32_t getDuration() const;
    uint32_t getPosition() const;
    bool isPlaying() const { return m_isPlaying; }

private:
    AudioFilesystem* m_filesystem;
    SAMEngine m_samEngine;
    
    // Playback state
    std::vector<int16_t> m_audioBuffer;
    size_t m_readPosition;
    bool m_isOpen;
    bool m_isPlaying;
    
    // Audio format
    AudioFormat m_format;
    uint32_t m_targetSampleRate;
    
    // Extensions
    static const char* s_extensions[];
    
    // Helper functions
    void clearBuffer();
    bool allocateBuffer(size_t samples);
    void initFormat();
};

#endif // AUDIO_CODEC_SAM_H