/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM SPEECH SYNTHESIS CODEC                                                  ║
 ║  Implements AudioCodec interface for text-to-speech                         ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#ifndef AUDIO_CODEC_SAM_H
#define AUDIO_CODEC_SAM_H

#include "AudioCodec.h"
#include "SAMEngine.h"

// Forward declaration
class AudioFilesystem;

class AudioCodec_SAM : public AudioCodec {
public:
    AudioCodec_SAM(AudioFilesystem* fs);
    ~AudioCodec_SAM();
    
    // AudioCodec pure virtual implementations
    const char* getName() override;
    const char* getVersion() override;
    const char** getExtensions() override;
    CodecCapabilities getCapabilities() override;
    
    bool probe(const char* filename) override;
    bool open(const char* filename) override;
    void close() override;
    bool isOpen() override;
    
    AudioFormat getFormat() override;
    size_t read(void* buffer, size_t size) override;
    bool seek(uint32_t position) override;
    
    void setTargetSampleRate(uint32_t rate) override;
    uint32_t getTargetSampleRate() override;
    
    // SAM-specific functions
    bool synthesizeText(const String& text);
    bool applyPreset(SAMVoicePreset preset);
    bool setVoiceParams(const SAMVoiceParams& params);
    void getVoiceParams(SAMVoiceParams& params) const;
    
    SAMEngine* getSAMEngine() { return &m_samEngine; }
    
private:
    AudioFilesystem* m_filesystem;
    SAMEngine m_samEngine;
    
    String m_currentText;
    std::vector<int16_t> m_audioBuffer;
    size_t m_bufferPosition;
    bool m_isOpen;
    uint32_t m_targetSampleRate;
    
    static const char* s_extensions[];
    
    void generateAudio();
};

#endif // AUDIO_CODEC_SAM_H