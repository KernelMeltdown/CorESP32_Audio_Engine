// AudioEngine.h - ESP32 Audio OS v1.9
// Core Audio Engine with LFO Modulation

#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <Arduino.h>
#include "AudioSettings.h"

// ESP32 variant detection for I2S
#if defined(CONFIG_IDF_TARGET_ESP32)
  #include <driver/i2s.h>
  #define USE_LEGACY_I2S 1
#else
  #include <driver/i2s_std.h>
  #define USE_LEGACY_I2S 0
#endif

// ============================================================================
// MIDI NOTE DEFINITIONS
// ============================================================================
#define NOTE_REST  0
#define NOTE_C3   48
#define NOTE_D3   50
#define NOTE_E3   52
#define NOTE_F3   53
#define NOTE_G3   55
#define NOTE_A3   57
#define NOTE_B3   59
#define NOTE_C4   60
#define NOTE_D4   62
#define NOTE_E4   64
#define NOTE_F4   65
#define NOTE_G4   67
#define NOTE_A4   69
#define NOTE_B4   71
#define NOTE_C5   72
#define NOTE_D5   74
#define NOTE_E5   76
#define NOTE_F5   77
#define NOTE_G5   79
#define NOTE_A5   81
#define NOTE_B5   83
#define NOTE_C6   84

// ============================================================================
// MELODY NOTE STRUCTURE
// ============================================================================
struct Note {
  uint8_t pitch;
  uint16_t duration;
  uint8_t velocity;
};

// ============================================================================
// FIXED-POINT TYPES
// ============================================================================
#if USE_FIXED_POINT_MATH
  typedef uint32_t fixed_point_t;
  #define FIXED_SHIFT 16
  #define FLOAT_TO_FIXED(f) ((fixed_point_t)((f) * 65536.0f))
  #define FIXED_TO_FLOAT(x) (((float)(x)) / 65536.0f)
  #define FIXED_MUL(a, b) (((int64_t)(a) * (int64_t)(b)) >> FIXED_SHIFT)
#endif

// ============================================================================
// ENVELOPE GENERATOR
// ============================================================================
struct Envelope {
  enum State { ENV_OFF, ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE };
  State state;
  uint32_t sampleCount;
  
  Envelope() : state(ENV_OFF), sampleCount(0) {}
  
  inline void on() {
    state = ENV_ATTACK;
    sampleCount = 0;
  }
  
  inline void off() {
    if (state != ENV_OFF) {
      state = ENV_RELEASE;
      sampleCount = 0;
    }
  }
  
  inline bool isActive() const {
    return state != ENV_OFF;
  }
  
  inline uint8_t get() {
    if (state == ENV_OFF) return 0;
    
    sampleCount++;
    
    switch(state) {
      case ENV_ATTACK:
        if (sampleCount >= ENV_ATTACK_SAMPLES) {
          state = ENV_DECAY;
          sampleCount = 0;
          return 255;
        }
        return (uint8_t)((sampleCount * 255) / ENV_ATTACK_SAMPLES);
        
      case ENV_DECAY:
        if (sampleCount >= ENV_DECAY_SAMPLES) {
          state = ENV_SUSTAIN;
          return ENV_SUSTAIN_LEVEL;
        }
        return (uint8_t)(255 - ((sampleCount * (255 - ENV_SUSTAIN_LEVEL)) / ENV_DECAY_SAMPLES));
        
      case ENV_SUSTAIN:
        return ENV_SUSTAIN_LEVEL;
        
      case ENV_RELEASE:
        if (sampleCount >= ENV_RELEASE_SAMPLES) {
          state = ENV_OFF;
          return 0;
        }
        return (uint8_t)(ENV_SUSTAIN_LEVEL - ((sampleCount * ENV_SUSTAIN_LEVEL) / ENV_RELEASE_SAMPLES));
        
      default:
        return 0;
    }
  }
};

// ============================================================================
// LFO OSCILLATOR (NEW!)
// ============================================================================
struct LFO {
  float phase;
  float phaseInc;
  
  LFO() : phase(0.0f), phaseInc(0.0f) {}
  
  void setRate(float rateHz, float sampleRate) {
    phaseInc = rateHz / sampleRate;
  }
  
  void reset() {
    phase = 0.0f;
  }
  
  // Get sine wave output (-1.0 to +1.0)
  inline float getSine() {
    float output = sinf(phase * 2.0f * PI);
    phase += phaseInc;
    if (phase >= 1.0f) phase -= 1.0f;
    return output;
  }
  
  // Get triangle wave output (-1.0 to +1.0)
  inline float getTriangle() {
    float output;
    if (phase < 0.5f) {
      output = (phase * 4.0f) - 1.0f;
    } else {
      output = 3.0f - (phase * 4.0f);
    }
    phase += phaseInc;
    if (phase >= 1.0f) phase -= 1.0f;
    return output;
  }
};

// ============================================================================
// VOICE STRUCTURE
// ============================================================================
struct Voice {
  bool on;
  uint8_t note;
  uint8_t vel;
  WaveformType waveform;
  
  #if USE_FIXED_POINT_MATH
    fixed_point_t phase;
    fixed_point_t phaseInc;
  #else
    float phase;
    float phaseInc;
  #endif
  
  Envelope env;
  
  Voice() : on(false), note(0), vel(127), waveform(WAVE_SINE), phase(0), phaseInc(0) {}
  
  void noteOn(uint8_t n, uint8_t v, uint32_t sampleRate);
  void noteOff();
  int16_t getSample(float lfoVibrato = 0.0f, float lfoTremolo = 1.0f);
};

// ============================================================================
// MELODY PLAYER
// ============================================================================
class MelodyPlayer {
private:
  const Note* melody;
  size_t melodyLen;
  size_t currentNote;
  uint32_t noteStartTime;
  bool playing;
  class AudioEngine* audio;
  
public:
  MelodyPlayer() : melody(nullptr), melodyLen(0), currentNote(0), 
                   noteStartTime(0), playing(false), audio(nullptr) {}
  
  void setAudioEngine(class AudioEngine* engine) { audio = engine; }
  void play(const Note* m, size_t len);
  void stop();
  void update();
  bool isPlaying() const { return playing; }
};

// ============================================================================
// MAIN AUDIO ENGINE CLASS
// ============================================================================
class AudioEngine {
private:
  AudioSettings* settings;
  Voice voices[8];
  uint8_t voiceCount;
  
  MelodyPlayer melodyPlayer;
  
  // LFO Oscillator (NEW!)
  LFO lfo;
  
  // Delay buffer
  int16_t* delayBuffer;
  uint32_t delayBufferSize;
  uint32_t delayWritePos;
  
  // Biquad EQ Filters
  struct BiquadFilter {
    float b0, b1, b2;
    float a1, a2;
    float x1[2], x2[2];
    float y1[2], y2[2];
    
    BiquadFilter() : b0(1), b1(0), b2(0), a1(0), a2(0) {
      for(int i=0; i<2; i++) {
        x1[i] = x2[i] = y1[i] = y2[i] = 0;
      }
    }
    
    inline float process(float input, int channel) {
      float output = b0 * input + b1 * x1[channel] + b2 * x2[channel]
                                - a1 * y1[channel] - a2 * y2[channel];
      x2[channel] = x1[channel];
      x1[channel] = input;
      y2[channel] = y1[channel];
      y1[channel] = output;
      return output;
    }
    
    void reset() {
      for(int i=0; i<2; i++) {
        x1[i] = x2[i] = y1[i] = y2[i] = 0;
      }
    }
  };
  
  BiquadFilter eqBass;
  BiquadFilter eqMid;
  BiquadFilter eqTreble;
  
  // State-Variable Filter
  struct StateVariableFilter {
    float lowpass[2];
    float bandpass[2];
    float highpass[2];
    float f;
    float q;
    
    StateVariableFilter() : f(0.1f), q(1.0f) {
      for(int i=0; i<2; i++) {
        lowpass[i] = bandpass[i] = highpass[i] = 0.0f;
      }
    }
    
    inline void process(float input, int channel, float& lp, float& bp, float& hp) {
      lowpass[channel] += f * bandpass[channel];
      highpass[channel] = input - lowpass[channel] - q * bandpass[channel];
      bandpass[channel] += f * highpass[channel];
      
      lp = lowpass[channel];
      bp = bandpass[channel];
      hp = highpass[channel];
    }
    
    void updateCoefficients(float cutoffHz, float resonance, float sampleRate) {
      f = 2.0f * sinf(PI * cutoffHz / sampleRate);
      q = 1.0f - resonance;
      if (f > 1.99f) f = 1.99f;
      if (q < 0.01f) q = 0.01f;
    }
    
    void reset() {
      for(int i=0; i<2; i++) {
        lowpass[i] = bandpass[i] = highpass[i] = 0.0f;
      }
    }
  };
  
  StateVariableFilter svf;
  
  // Schroeder Reverb Structure
  struct SchroederReverb {
    struct CombFilter {
      float* buffer;
      uint32_t bufferSize;
      uint32_t readPos;
      float feedback;
      float filterStore;
      
      CombFilter() : buffer(nullptr), bufferSize(0), readPos(0), 
                     feedback(0.5f), filterStore(0.0f) {}
      
      float process(float input, float damping) {
        float output = buffer[readPos];
        filterStore = (output * (1.0f - damping)) + (filterStore * damping);
        buffer[readPos] = input + (filterStore * feedback);
        readPos++;
        if (readPos >= bufferSize) readPos = 0;
        return output;
      }
      
      void reset() {
        if (buffer) {
          memset(buffer, 0, bufferSize * sizeof(float));
        }
        readPos = 0;
        filterStore = 0.0f;
      }
    };
    
    struct AllpassFilter {
      float* buffer;
      uint32_t bufferSize;
      uint32_t readPos;
      
      AllpassFilter() : buffer(nullptr), bufferSize(0), readPos(0) {}
      
      float process(float input) {
        float bufferOut = buffer[readPos];
        float output = -input + bufferOut;
        buffer[readPos] = input + (bufferOut * 0.5f);
        readPos++;
        if (readPos >= bufferSize) readPos = 0;
        return output;
      }
      
      void reset() {
        if (buffer) {
          memset(buffer, 0, bufferSize * sizeof(float));
        }
        readPos = 0;
      }
    };
    
    CombFilter comb[4];
    AllpassFilter allpass[2];
    float* reverbBuffer;
    bool initialized;
    
    SchroederReverb() : reverbBuffer(nullptr), initialized(false) {}
    
    bool init() {
      reverbBuffer = (float*)malloc(REVERB_BUFFER_SIZE * sizeof(float));
      if (!reverbBuffer) return false;
      
      memset(reverbBuffer, 0, REVERB_BUFFER_SIZE * sizeof(float));
      
      uint32_t offset = 0;
      comb[0].buffer = reverbBuffer + offset;
      comb[0].bufferSize = REVERB_COMB1_DELAY;
      offset += REVERB_COMB1_DELAY;
      
      comb[1].buffer = reverbBuffer + offset;
      comb[1].bufferSize = REVERB_COMB2_DELAY;
      offset += REVERB_COMB2_DELAY;
      
      comb[2].buffer = reverbBuffer + offset;
      comb[2].bufferSize = REVERB_COMB3_DELAY;
      offset += REVERB_COMB3_DELAY;
      
      comb[3].buffer = reverbBuffer + offset;
      comb[3].bufferSize = REVERB_COMB4_DELAY;
      offset += REVERB_COMB4_DELAY;
      
      allpass[0].buffer = reverbBuffer + offset;
      allpass[0].bufferSize = REVERB_ALLPASS1_DELAY;
      offset += REVERB_ALLPASS1_DELAY;
      
      allpass[1].buffer = reverbBuffer + offset;
      allpass[1].bufferSize = REVERB_ALLPASS2_DELAY;
      
      initialized = true;
      return true;
    }
    
    void deinit() {
      if (reverbBuffer) {
        free(reverbBuffer);
        reverbBuffer = nullptr;
      }
      initialized = false;
    }
    
    void updateParameters(float roomSize, float damping) {
      float feedback = 0.5f + (roomSize * 0.45f);
      for (int i = 0; i < 4; i++) {
        comb[i].feedback = feedback;
      }
    }
    
    float process(float input, float damping) {
      float combOut = 0.0f;
      for (int i = 0; i < 4; i++) {
        combOut += comb[i].process(input, damping);
      }
      combOut *= 0.25f;
      
      float output = allpass[0].process(combOut);
      output = allpass[1].process(output);
      
      return output;
    }
    
    void reset() {
      for (int i = 0; i < 4; i++) {
        comb[i].reset();
      }
      for (int i = 0; i < 2; i++) {
        allpass[i].reset();
      }
    }
  };
  
  SchroederReverb reverb;
  
  // I2S handles
  #if USE_LEGACY_I2S
    // ESP32 Classic
  #else
    i2s_chan_handle_t tx_handle;
  #endif
  
  TaskHandle_t audioTaskHandle;
  bool initialized;
  bool pwmActive;
  
  // Performance monitoring
  uint32_t lastCPUCheck;
  uint32_t audioTaskCount;
  float cpuUsage;
  
  // Internal methods
  void initI2S();
  void initPWM();
  void deinitI2S();
  void deinitPWM();
  bool allocateDelayBuffer();
  void freeDelayBuffer();
  
  // Audio task (I2S mode)
  static void audioTask(void* parameter);
  
  // Voice management
  int findFreeVoice();
  
public:
  AudioEngine();
  ~AudioEngine();
  
  // Initialization
  bool init(AudioSettings* settings);
  void deinit();
  
  // Update (call in loop for PWM mode)
  void update();
  
  // Playback control
  void noteOn(uint8_t note, uint8_t velocity = 127);
  void noteOff(uint8_t note);
  void allNotesOff();
  
  void playMelody(const Note* melody, size_t length);
  void stopMelody();
  bool isPlaying();
  
  // Settings
  void setVolume(uint8_t volume);
  uint8_t getVolume();
  
  void setEQ(int8_t bass, int8_t mid, int8_t treble);
  void getEQ(int8_t& bass, int8_t& mid, int8_t& treble);
  void setEQEnabled(bool enabled);
  bool getEQEnabled();
  void updateEQFilters();
  
  // Filter control
  void setFilterEnabled(bool enabled);
  bool getFilterEnabled();
  void setFilterType(FilterType type);
  FilterType getFilterType();
  const char* getFilterTypeName();
  void setFilterCutoff(float cutoffHz);
  float getFilterCutoff();
  void setFilterResonance(float resonance);
  float getFilterResonance();
  void updateFilterCoefficients();
  
  // Reverb control
  void setReverbEnabled(bool enabled);
  bool getReverbEnabled();
  void setReverbRoomSize(float size);
  float getReverbRoomSize();
  void setReverbDamping(float damping);
  float getReverbDamping();
  void setReverbWet(float wet);
  float getReverbWet();
  void updateReverbParameters();
  
  // LFO control (NEW!)
  void setLFOEnabled(bool enabled);
  bool getLFOEnabled();
  void setLFOVibratoEnabled(bool enabled);
  bool getLFOVibratoEnabled();
  void setLFOTremoloEnabled(bool enabled);
  bool getLFOTremoloEnabled();
  void setLFORate(float rateHz);
  float getLFORate();
  void setLFODepth(float depthPercent);
  float getLFODepth();
  void updateLFORate();
  
  // Delay control
  void setDelayEnabled(bool enabled);
  bool getDelayEnabled();
  void setDelayTime(uint16_t ms);
  uint16_t getDelayTime();
  void setDelayFeedback(uint8_t percent);
  uint8_t getDelayFeedback();
  void setDelayMix(uint8_t percent);
  uint8_t getDelayMix();
  
  // Waveform control
  void setWaveform(WaveformType waveform);
  WaveformType getWaveform();
  const char* getWaveformName();
  
  // Status
  uint8_t getActiveVoices();
  uint8_t getVoiceCount() { return voiceCount; }
  uint32_t getSampleRate() { return settings->sampleRate; }
  const char* getModeName() { return settings->getModeName(); }
  AudioSettings* getSettings() { return settings; }
  
  // Performance monitoring
  float getCPUUsage() { return cpuUsage; }
  uint32_t getFreeHeap() { return ESP.getFreeHeap(); }
  
  // Wavetable initialization
  static void initWavetable();
  
  // Access to melody player
  MelodyPlayer* getMelodyPlayer() { return &melodyPlayer; }
};

// ============================================================================
// GLOBAL WAVETABLE
// ============================================================================
#if USE_WAVETABLE_LOOKUP
  extern int16_t sineTable[WAVETABLE_SIZE];
#endif

#endif // AUDIO_ENGINE_H
