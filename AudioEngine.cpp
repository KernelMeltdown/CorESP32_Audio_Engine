// AudioEngine.cpp - ESP32 Audio OS v1.9

#include "AudioEngine.h"
#include "AudioConfig.h"
#include <math.h>

// ============================================================================
// WAVETABLE STORAGE
// ============================================================================
#if USE_WAVETABLE_LOOKUP
  int16_t sineTable[WAVETABLE_SIZE];
#endif

// ============================================================================
// WAVETABLE INITIALIZATION
// ============================================================================
void AudioEngine::initWavetable() {
  #if USE_WAVETABLE_LOOKUP
    Serial.println(F("[AUDIO] Generating wavetable..."));
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
      float phase = (float)i / WAVETABLE_SIZE;
      sineTable[i] = (int16_t)(sin(phase * 2.0f * PI) * 32767.0f);
    }
    Serial.println(F("[AUDIO] ✓ Wavetable ready"));
  #endif
}

// ============================================================================
// VOICE IMPLEMENTATION (WITH LFO SUPPORT!)
// ============================================================================

void Voice::noteOn(uint8_t n, uint8_t v, uint32_t sampleRate) {
  note = n;
  vel = v;
  phase = 0;
  
  float freq = 440.0f * powf(2.0f, (n - 69) / 12.0f);
  
  #if USE_FIXED_POINT_MATH
    float phaseIncFloat = freq / (float)sampleRate;
    phaseInc = FLOAT_TO_FIXED(phaseIncFloat);
  #else
    phaseInc = freq / (float)sampleRate;
  #endif
  
  on = true;
  env.on();
}

void Voice::noteOff() {
  env.off();
}

int16_t Voice::getSample(float lfoVibrato, float lfoTremolo) {
  if (!on) return 0;
  
  uint8_t envValue = env.get();
  if (!env.isActive()) {
    on = false;
    return 0;
  }
  
  int16_t sample = 0;
  
  // Apply LFO Vibrato (pitch modulation)
  #if USE_FIXED_POINT_MATH
    fixed_point_t modulatedPhaseInc = phaseInc;
    if (lfoVibrato != 0.0f) {
      float phaseIncFloat = FIXED_TO_FLOAT(phaseInc);
      phaseIncFloat *= (1.0f + lfoVibrato);
      modulatedPhaseInc = FLOAT_TO_FIXED(phaseIncFloat);
    }
    float phaseFloat = FIXED_TO_FLOAT(phase);
  #else
    float modulatedPhaseInc = phaseInc;
    if (lfoVibrato != 0.0f) {
      modulatedPhaseInc = phaseInc * (1.0f + lfoVibrato);
    }
    float phaseFloat = phase;
  #endif
  
  switch(waveform) {
    case WAVE_SINE:
      #if USE_WAVETABLE_LOOKUP && USE_FIXED_POINT_MATH
        {
          uint32_t index = (phase >> FIXED_SHIFT) & (WAVETABLE_SIZE - 1);
          uint32_t nextIndex = (index + 1) & (WAVETABLE_SIZE - 1);
          uint32_t frac = (phase >> 8) & 0xFF;
          
          int16_t s0 = sineTable[index];
          int16_t s1 = sineTable[nextIndex];
          
          int32_t interpolated = s0 + (((s1 - s0) * frac) >> 8);
          sample = (int16_t)interpolated;
        }
      #elif USE_WAVETABLE_LOOKUP
        {
          uint32_t index = (uint32_t)(phaseFloat * WAVETABLE_SIZE) & (WAVETABLE_SIZE - 1);
          sample = sineTable[index];
        }
      #else
        sample = (int16_t)(sin(phaseFloat * 2.0f * PI) * 32767.0f);
      #endif
      break;
      
    case WAVE_SQUARE:
      sample = (phaseFloat < 0.5f) ? 32767 : -32767;
      break;
      
    case WAVE_SAWTOOTH:
      sample = (int16_t)((phaseFloat * 2.0f - 1.0f) * 32767.0f);
      break;
      
    case WAVE_TRIANGLE:
      if (phaseFloat < 0.5f) {
        sample = (int16_t)((phaseFloat * 4.0f - 1.0f) * 32767.0f);
      } else {
        sample = (int16_t)((3.0f - phaseFloat * 4.0f) * 32767.0f);
      }
      break;
      
    case WAVE_NOISE:
      {
        static uint32_t lfsr = 0xACE1u;
        uint32_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 22) ^ (lfsr >> 31)) & 1;
        lfsr = (lfsr >> 1) | (bit << 31);
        sample = (int16_t)((lfsr & 0xFFFF) - 32768);
      }
      break;
  }
  
  // Advance phase with vibrato modulation
  #if USE_FIXED_POINT_MATH
    phase += modulatedPhaseInc;
    if (phase >= FLOAT_TO_FIXED(1.0f)) {
      phase -= FLOAT_TO_FIXED(1.0f);
    }
  #else
    phase += modulatedPhaseInc;
    if (phase >= 1.0f) phase -= 1.0f;
  #endif
  
  // Apply envelope
  int32_t result = (int32_t)sample * envValue / 255;
  result = result * vel / 127;
  
  // Apply LFO Tremolo (amplitude modulation)
  if (lfoTremolo != 1.0f) {
    result = (int32_t)(result * lfoTremolo);
  }
  
  return (int16_t)constrain(result, -32768, 32767);
}

// ============================================================================
// MELODY PLAYER IMPLEMENTATION
// ============================================================================

void MelodyPlayer::play(const Note* m, size_t len) {
  // Clean up old melody if we own it
  if (ownsMemory && melody) {
    delete[] melody;
    melody = nullptr;
  }

  // Copy melody data (take ownership)
  melody = new Note[len];
  if (!melody) {
    playing = false;
    return;
  }

  memcpy(melody, m, len * sizeof(Note));
  melodyLen = len;
  ownsMemory = true;
  currentNote = 0;
  playing = true;
  noteStartTime = millis();

  if (audio && melody[0].pitch != NOTE_REST) {
    audio->noteOn(melody[0].pitch, melody[0].velocity);
  }
}


void MelodyPlayer::stop() {
  playing = false;
  if (audio) {
    audio->allNotesOff();
  }

  // Clean up melody memory
  if (ownsMemory && melody) {
    delete[] melody;
    melody = nullptr;
    ownsMemory = false;
  }
}


void MelodyPlayer::update() {
  if (!playing || !melody || !audio) return;
  
  uint32_t now = millis();
  
  if (now - noteStartTime >= melody[currentNote].duration) {
    if (melody[currentNote].pitch != NOTE_REST) {
      audio->noteOff(melody[currentNote].pitch);
    }
    
    currentNote++;
    if (currentNote >= melodyLen) {
      playing = false;
      return;
    }
    
    noteStartTime = now;
    if (melody[currentNote].pitch != NOTE_REST) {
      audio->noteOn(melody[currentNote].pitch, melody[currentNote].velocity);
    }
  }
}

// ============================================================================
// AUDIO ENGINE IMPLEMENTATION
// ============================================================================

AudioEngine::AudioEngine() 
  : settings(nullptr), voiceCount(0), audioTaskHandle(nullptr), 
    initialized(false), pwmActive(false), lastCPUCheck(0), 
    audioTaskCount(0), cpuUsage(0.0f),
    delayBuffer(nullptr), delayBufferSize(0), delayWritePos(0) {
  #if !USE_LEGACY_I2S
    tx_handle = nullptr;
  #endif
}

AudioEngine::~AudioEngine() {
  deinit();
}

bool AudioEngine::init(AudioSettings* cfg) {
  if (initialized) {
    Serial.println(F("[AUDIO] Already initialized"));
    return false;
  }
  
  settings = cfg;
  voiceCount = settings->voices;
  
  Serial.println(F("[AUDIO] Initializing..."));
  
  #if USE_WAVETABLE_LOOKUP
    initWavetable();
  #endif
  
  for (int i = 0; i < voiceCount; i++) {
    voices[i] = Voice();
    voices[i].waveform = settings->waveform;
  }
  
  // Conditional delay buffer allocation
  if (settings->delay.enabled) {
    if (!allocateDelayBuffer()) {
      Serial.println(F("[WARN] Delay buffer allocation failed - delay disabled"));
      settings->delay.enabled = false;
    }
  } else {
    Serial.println(F("[AUDIO] ✓ Delay buffer not allocated (disabled)"));
  }
  
  // Initialize Biquad EQ filters
  updateEQFilters();
  Serial.println(F("[AUDIO] ✓ Biquad EQ filters initialized"));
  
  // Initialize State-Variable Filter
  updateFilterCoefficients();
  Serial.println(F("[AUDIO] ✓ State-Variable Filter initialized"));
  
  // Initialize Schroeder Reverb
  if (settings->reverb.enabled) {
    if (reverb.init()) {
      updateReverbParameters();
      Serial.println(F("[AUDIO] ✓ Schroeder Reverb initialized"));
    } else {
      Serial.println(F("[ERROR] Reverb buffer allocation failed - reverb disabled"));
      settings->reverb.enabled = false;
    }
  } else {
    Serial.println(F("[AUDIO] ✓ Reverb not allocated (disabled)"));
  }
  
  // Initialize LFO (NEW!)
  updateLFORate();
  Serial.println(F("[AUDIO] ✓ LFO initialized"));
  
  melodyPlayer.setAudioEngine(this);
  
  if (settings->mode == MODE_I2S) {
    initI2S();
  } else {
    initPWM();
  }
  
  initialized = true;
  
  Serial.printf("[AUDIO] ✓ Mode: %s, Sample Rate: %u Hz, Voices: %u\n",
                settings->getModeName(), settings->sampleRate, voiceCount);
  
  #if USE_FIXED_POINT_MATH
    Serial.println(F("[AUDIO] ✓ Fixed-point math enabled"));
  #endif
  #if USE_WAVETABLE_LOOKUP
    Serial.println(F("[AUDIO] ✓ Wavetable lookup enabled"));
  #endif
  
  Serial.printf("[AUDIO] ✓ Waveform: %s\n", getWaveformName());
  Serial.printf("[AUDIO] ✓ Free RAM: %d KB\n", ESP.getFreeHeap() / 1024);
  
  #if HAS_DUAL_CORE
    if (settings->multiCore.useDualCore) {
      Serial.printf("[AUDIO] ✓ Dual-core: Audio=%u, UI=%u\n", 
                    settings->multiCore.audioCore, settings->multiCore.uiCore);
    }
  #endif
  
  return true;
}

void AudioEngine::deinit() {
  if (!initialized) return;
  
  Serial.println(F("[AUDIO] Shutting down..."));
  
  if (settings->mode == MODE_I2S) {
    deinitI2S();
  } else {
    deinitPWM();
  }
  
  freeDelayBuffer();
  reverb.deinit();
  
  initialized = false;
  Serial.println(F("[AUDIO] ✓ Shutdown complete"));
}

// ============================================================================
// CONDITIONAL DELAY BUFFER ALLOCATION
// ============================================================================

bool AudioEngine::allocateDelayBuffer() {
  delayBufferSize = (settings->sampleRate * MAX_DELAY_TIME) / 1000;
  delayBuffer = (int16_t*)malloc(delayBufferSize * sizeof(int16_t));
  
  if (!delayBuffer) {
    delayBufferSize = 0;
    return false;
  }
  
  memset(delayBuffer, 0, delayBufferSize * sizeof(int16_t));
  delayWritePos = 0;
  Serial.printf("[AUDIO] ✓ Delay buffer: %u samples (%.1f KB)\n", 
                delayBufferSize, (delayBufferSize * sizeof(int16_t)) / 1024.0f);
  return true;
}

void AudioEngine::freeDelayBuffer() {
  if (delayBuffer) {
    free(delayBuffer);
    delayBuffer = nullptr;
    delayBufferSize = 0;
    Serial.println(F("[AUDIO] ✓ Delay buffer freed"));
  }
}

// ============================================================================
// BIQUAD EQ FILTER CALCULATION
// ============================================================================

void AudioEngine::updateEQFilters() {
  float fs = (float)settings->sampleRate;
  
  // Calculate Bass filter
  if (settings->eq.bass != 0) {
    float freq = (float)settings->eq.bassFreq;
    float Q = settings->eq.q;
    float gain = (float)settings->eq.bass;
    float A = powf(10.0f, gain / 40.0f);
    float omega = 2.0f * PI * freq / fs;
    float sn = sinf(omega);
    float cs = cosf(omega);
    float alpha = sn / (2.0f * Q);
    
    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cs;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cs;
    float a2 = 1.0f - alpha / A;
    
    eqBass.b0 = b0 / a0;
    eqBass.b1 = b1 / a0;
    eqBass.b2 = b2 / a0;
    eqBass.a1 = a1 / a0;
    eqBass.a2 = a2 / a0;
  } else {
    eqBass.b0 = 1.0f;
    eqBass.b1 = 0.0f;
    eqBass.b2 = 0.0f;
    eqBass.a1 = 0.0f;
    eqBass.a2 = 0.0f;
  }
  
  // Calculate Mid filter
  if (settings->eq.mid != 0) {
    float freq = (float)settings->eq.midFreq;
    float Q = settings->eq.q;
    float gain = (float)settings->eq.mid;
    float A = powf(10.0f, gain / 40.0f);
    float omega = 2.0f * PI * freq / fs;
    float sn = sinf(omega);
    float cs = cosf(omega);
    float alpha = sn / (2.0f * Q);
    
    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cs;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cs;
    float a2 = 1.0f - alpha / A;
    
    eqMid.b0 = b0 / a0;
    eqMid.b1 = b1 / a0;
    eqMid.b2 = b2 / a0;
    eqMid.a1 = a1 / a0;
    eqMid.a2 = a2 / a0;
  } else {
    eqMid.b0 = 1.0f;
    eqMid.b1 = 0.0f;
    eqMid.b2 = 0.0f;
    eqMid.a1 = 0.0f;
    eqMid.a2 = 0.0f;
  }
  
  // Calculate Treble filter
  if (settings->eq.treble != 0) {
    float freq = (float)settings->eq.trebleFreq;
    float Q = settings->eq.q;
    float gain = (float)settings->eq.treble;
    float A = powf(10.0f, gain / 40.0f);
    float omega = 2.0f * PI * freq / fs;
    float sn = sinf(omega);
    float cs = cosf(omega);
    float alpha = sn / (2.0f * Q);
    
    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cs;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cs;
    float a2 = 1.0f - alpha / A;
    
    eqTreble.b0 = b0 / a0;
    eqTreble.b1 = b1 / a0;
    eqTreble.b2 = b2 / a0;
    eqTreble.a1 = a1 / a0;
    eqTreble.a2 = a2 / a0;
  } else {
    eqTreble.b0 = 1.0f;
    eqTreble.b1 = 0.0f;
    eqTreble.b2 = 0.0f;
    eqTreble.a1 = 0.0f;
    eqTreble.a2 = 0.0f;
  }
  
  eqBass.reset();
  eqMid.reset();
  eqTreble.reset();
}

// ============================================================================
// STATE-VARIABLE FILTER CALCULATION
// ============================================================================

void AudioEngine::updateFilterCoefficients() {
  svf.updateCoefficients(
    settings->filter.cutoff,
    settings->filter.resonance,
    (float)settings->sampleRate
  );
  svf.reset();
}

// ============================================================================
// SCHROEDER REVERB PARAMETER UPDATE
// ============================================================================

void AudioEngine::updateReverbParameters() {
  if (reverb.initialized) {
    reverb.updateParameters(settings->reverb.roomSize, settings->reverb.damping);
  }
}

// ============================================================================
// LFO RATE UPDATE (NEW!)
// ============================================================================

void AudioEngine::updateLFORate() {
  lfo.setRate(settings->lfo.rate, (float)settings->sampleRate);
}
// ============================================================================
// I2S INITIALIZATION
// ============================================================================

void AudioEngine::initI2S() {
  Serial.println(F("[I2S] Initializing..."));
  
  #if USE_LEGACY_I2S
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = settings->sampleRate,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = settings->performance.i2sNumBuffers,
      .dma_buf_len = settings->performance.i2sBufferSize,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_PIN_NO_CHANGE,
      .ws_io_num = I2S_PIN_NO_CHANGE,
      .data_out_num = settings->i2s.pin,
      .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
      Serial.printf("[ERROR] I2S driver install failed: %d\n", err);
      return;
    }
    
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
      Serial.printf("[ERROR] I2S set pin failed: %d\n", err);
      i2s_driver_uninstall(I2S_NUM_0);
      return;
    }
    
  #else
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = settings->performance.i2sNumBuffers;
    chan_cfg.dma_frame_num = settings->performance.i2sBufferSize;
    chan_cfg.auto_clear = true;
    
    esp_err_t err = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
    if (err != ESP_OK) {
      Serial.printf("[ERROR] I2S new channel failed: %d\n", err);
      return;
    }
    
    i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(settings->sampleRate),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
      .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = I2S_GPIO_UNUSED,
        .ws = I2S_GPIO_UNUSED,
        .dout = (gpio_num_t)settings->i2s.pin,
        .din = I2S_GPIO_UNUSED,
        .invert_flags = {
          .mclk_inv = false,
          .bclk_inv = false,
          .ws_inv = false
        }
      }
    };
    
    err = i2s_channel_init_std_mode(tx_handle, &std_cfg);
    if (err != ESP_OK) {
      Serial.printf("[ERROR] I2S init std mode failed: %d\n", err);
      i2s_del_channel(tx_handle);
      tx_handle = nullptr;
      return;
    }
    
    err = i2s_channel_enable(tx_handle);
    if (err != ESP_OK) {
      Serial.printf("[ERROR] I2S enable failed: %d\n", err);
      i2s_del_channel(tx_handle);
      tx_handle = nullptr;
      return;
    }
  #endif
  
  uint8_t audioCore = settings->multiCore.useDualCore ? settings->multiCore.audioCore : 0;
  
  BaseType_t result = xTaskCreatePinnedToCore(
    audioTask,
    "AudioTask",
    8192,
    this,
    configMAX_PRIORITIES - 1,
    &audioTaskHandle,
    audioCore
  );
  
  if (result != pdPASS) {
    Serial.println(F("[ERROR] Failed to create audio task"));
    deinitI2S();
    return;
  }
  
  Serial.printf("[I2S] ✓ Initialized on GPIO %u (Core %u)\n", settings->i2s.pin, audioCore);
}

void AudioEngine::deinitI2S() {
  if (audioTaskHandle) {
    vTaskDelete(audioTaskHandle);
    audioTaskHandle = nullptr;
  }
  
  #if USE_LEGACY_I2S
    i2s_driver_uninstall(I2S_NUM_0);
  #else
    if (tx_handle) {
      i2s_channel_disable(tx_handle);
      i2s_del_channel(tx_handle);
      tx_handle = nullptr;
    }
  #endif
}

// ============================================================================
// I2S AUDIO TASK (WITH LFO, REVERB, SVF, EQ & DELAY)
// ============================================================================

void AudioEngine::audioTask(void* parameter) {
  AudioEngine* engine = (AudioEngine*)parameter;
  int16_t buffer[engine->settings->performance.i2sBufferSize * 2];
  size_t bytesWritten;
  
  uint32_t taskCount = 0;
  uint32_t lastMonitor = millis();
  uint32_t lastMelodyUpdate = millis();
  
  while (true) {
    uint32_t now = millis();
    if (now - lastMelodyUpdate >= 1) {
      lastMelodyUpdate = now;
      engine->melodyPlayer.update();
    }
    
    for (uint32_t i = 0; i < engine->settings->performance.i2sBufferSize; i++) {
      // LFO outputs (per sample)
      float lfoVibrato = 0.0f;
      float lfoTremolo = 1.0f;
      if (engine->settings->lfo.enabled) {
        float lfoValue = engine->lfo.getSine(); // -1..1
        
        // Vibrato: pitch modulation (scaled by depth)
        if (engine->settings->lfo.vibratoEnabled) {
          float depth = engine->settings->lfo.depth / 100.0f;
          lfoVibrato = lfoValue * depth * 0.02f; // ±2% Pitch bei 100% Depth
        }
        
        // Tremolo: amplitude modulation (0..1)
        if (engine->settings->lfo.tremoloEnabled) {
          float depth = engine->settings->lfo.depth / 100.0f;
          // Map -1..1 to 1-depth .. 1
          float amp = 1.0f - depth + (lfoValue + 1.0f) * 0.5f * depth;
          lfoTremolo = amp;
        }
      }
      
      // Voice mixing
      int32_t mixed = 0;
      int active = 0;
      
      for (int v = 0; v < engine->voiceCount; v++) {
        if (engine->voices[v].on) {
          mixed += engine->voices[v].getSample(lfoVibrato, lfoTremolo);
          active++;
        }
      }
      
      if (active > 1) {
        mixed /= active;
      }
      
      mixed = (mixed * engine->settings->volume) / 255;
      
      // State-Variable Filter
      if (engine->settings->filter.enabled) {
        float sample = (float)mixed;
        float lp, bp, hp;
        
        engine->svf.process(sample, 0, lp, bp, hp);
        
        switch(engine->settings->filter.type) {
          case FILTER_LOWPASS:  sample = lp; break;
          case FILTER_HIGHPASS: sample = hp; break;
          case FILTER_BANDPASS: sample = bp; break;
        }
        
        mixed = (int32_t)sample;
      }
      
      // Biquad EQ
      if (engine->settings->eq.enabled) {
        float sample = (float)mixed;
        
        if (engine->settings->eq.bass != 0) {
          sample = engine->eqBass.process(sample, 0);
        }
        if (engine->settings->eq.mid != 0) {
          sample = engine->eqMid.process(sample, 0);
        }
        if (engine->settings->eq.treble != 0) {
          sample = engine->eqTreble.process(sample, 0);
        }
        
        mixed = (int32_t)sample;
      }
      
      // Reverb
      if (engine->settings->reverb.enabled && engine->reverb.initialized) {
        float sample = (float)mixed;
        float wet = engine->reverb.process(sample, engine->settings->reverb.damping);
        float dry = sample * (1.0f - engine->settings->reverb.wet);
        float reverbOut = dry + (wet * engine->settings->reverb.wet);
        mixed = (int32_t)reverbOut;
      }
      
      // Delay
      if (engine->settings->delay.enabled && engine->delayBuffer) {
        uint32_t delaySamples = (engine->settings->sampleRate * engine->settings->delay.timeMs) / 1000;
        if (delaySamples >= engine->delayBufferSize) {
          delaySamples = engine->delayBufferSize - 1;
        }
        
        uint32_t readPos = (engine->delayWritePos + engine->delayBufferSize - delaySamples) % engine->delayBufferSize;
        int32_t delayedSample = engine->delayBuffer[readPos];
        
        int32_t feedbackAmount = (engine->settings->delay.feedback * delayedSample) / 100;
        int32_t toBuffer = mixed + feedbackAmount;
        
        if (toBuffer > 32767) toBuffer = 32767;
        if (toBuffer < -32768) toBuffer = -32768;
        engine->delayBuffer[engine->delayWritePos] = (int16_t)toBuffer;
        
        int32_t dry = (mixed * (100 - engine->settings->delay.mix)) / 100;
        int32_t wet = (delayedSample * engine->settings->delay.mix) / 100;
        mixed = dry + wet;
        
        engine->delayWritePos = (engine->delayWritePos + 1) % engine->delayBufferSize;
      }
      
      // Clipping
      if (mixed > 32767) mixed = 32767;
      if (mixed < -32768) mixed = -32768;
      
      buffer[i * 2] = (int16_t)mixed;
      buffer[i * 2 + 1] = (int16_t)mixed;
    }
    
    #if USE_LEGACY_I2S
      i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
    #else
      i2s_channel_write(engine->tx_handle, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
    #endif
    
    taskCount++;
    if (engine->settings->performance.enableCPUMonitor) {
      now = millis();
      if (now - lastMonitor >= CPU_MONITOR_INTERVAL) {
        engine->audioTaskCount = taskCount;
        taskCount = 0;
        lastMonitor = now;
        
        uint32_t expectedTasks = (CPU_MONITOR_INTERVAL * engine->settings->sampleRate) / engine->settings->performance.i2sBufferSize;
        engine->cpuUsage = (float)engine->audioTaskCount / expectedTasks * 100.0f;
      }
    }
  }
}

// ============================================================================
// PWM INITIALIZATION
// ============================================================================

void AudioEngine::initPWM() {
  Serial.println(F("[PWM] Initializing..."));
  
  ledcAttach(settings->pwm.pin, settings->pwm.frequency, settings->pwm.resolution);
  ledcWrite(settings->pwm.pin, 1 << (settings->pwm.resolution - 1));
  pwmActive = true;
  
  Serial.printf("[PWM] ✓ Initialized on GPIO %u (%u Hz, %u-bit)\n",
                settings->pwm.pin, settings->pwm.frequency, settings->pwm.resolution);
}

void AudioEngine::deinitPWM() {
  if (pwmActive) {
    ledcWrite(settings->pwm.pin, 0);
    ledcDetach(settings->pwm.pin);
    pinMode(settings->pwm.pin, OUTPUT);
    digitalWrite(settings->pwm.pin, LOW);
    pwmActive = false;
  }
}
// ============================================================================
// UPDATE (for PWM mode with LFO, REVERB, SVF, EQ & DELAY)
// ============================================================================

void AudioEngine::update() {
  if (settings->mode != MODE_PWM) return;
  
  static uint32_t lastMicros = 0;
  uint32_t now = micros();
  
  uint32_t interval = 1000000 / settings->sampleRate;
  
  if (now - lastMicros >= interval) {
    lastMicros = now;
    
    // LFO outputs (per sample)
    float lfoVibrato = 0.0f;
    float lfoTremolo = 1.0f;
    if (settings->lfo.enabled) {
      float lfoValue = lfo.getSine(); // -1..1
      
      if (settings->lfo.vibratoEnabled) {
        float depth = settings->lfo.depth / 100.0f;
        lfoVibrato = lfoValue * depth * 0.02f;
      }
      
      if (settings->lfo.tremoloEnabled) {
        float depth = settings->lfo.depth / 100.0f;
        float amp = 1.0f - depth + (lfoValue + 1.0f) * 0.5f * depth;
        lfoTremolo = amp;
      }
    }
    
    int32_t mixed = 0;
    int active = 0;
    
    for (int v = 0; v < voiceCount; v++) {
      if (voices[v].on) {
        mixed += voices[v].getSample(lfoVibrato, lfoTremolo);
        active++;
      }
    }
    
    if (active == 0) {
      if (pwmActive) {
        ledcWrite(settings->pwm.pin, 0);
        ledcDetach(settings->pwm.pin);
        pinMode(settings->pwm.pin, OUTPUT);
        digitalWrite(settings->pwm.pin, LOW);
        pwmActive = false;
      }
      melodyPlayer.update();
      return;
    }
    
    if (!pwmActive) {
      ledcAttach(settings->pwm.pin, settings->pwm.frequency, settings->pwm.resolution);
      pwmActive = true;
    }
    
    if (active > 1) {
      mixed /= active;
    }
    
    // SVF
    if (settings->filter.enabled) {
      float sample = (float)mixed;
      float lp, bp, hp;
      
      svf.process(sample, 0, lp, bp, hp);
      
      switch(settings->filter.type) {
        case FILTER_LOWPASS:  sample = lp; break;
        case FILTER_HIGHPASS: sample = hp; break;
        case FILTER_BANDPASS: sample = bp; break;
      }
      
      mixed = (int32_t)sample;
    }
    
    // EQ
    if (settings->eq.enabled) {
      float sample = (float)mixed;
      
      if (settings->eq.bass != 0)   sample = eqBass.process(sample, 0);
      if (settings->eq.mid != 0)    sample = eqMid.process(sample, 0);
      if (settings->eq.treble != 0) sample = eqTreble.process(sample, 0);
      
      mixed = (int32_t)sample;
    }
    
    // Reverb
    if (settings->reverb.enabled && reverb.initialized) {
      float sample = (float)mixed;
      float wet = reverb.process(sample, settings->reverb.damping);
      float dry = sample * (1.0f - settings->reverb.wet);
      float reverbOut = dry + (wet * settings->reverb.wet);
      mixed = (int32_t)reverbOut;
    }
    
    // Delay
    if (settings->delay.enabled && delayBuffer) {
      uint32_t delaySamples = (settings->sampleRate * settings->delay.timeMs) / 1000;
      if (delaySamples >= delayBufferSize) delaySamples = delayBufferSize - 1;
      
      uint32_t readPos = (delayWritePos + delayBufferSize - delaySamples) % delayBufferSize;
      int32_t delayedSample = delayBuffer[readPos];
      
      int32_t feedbackAmount = (settings->delay.feedback * delayedSample) / 100;
      int32_t toBuffer = mixed + feedbackAmount;
      
      if (toBuffer > 32767) toBuffer = 32767;
      if (toBuffer < -32768) toBuffer = -32768;
      delayBuffer[delayWritePos] = (int16_t)toBuffer;
      
      int32_t dry = (mixed * (100 - settings->delay.mix)) / 100;
      int32_t wet = (delayedSample * settings->delay.mix) / 100;
      mixed = dry + wet;
      
      delayWritePos = (delayWritePos + 1) % delayBufferSize;
    }
    
    mixed = (mixed * settings->pwm.gain * settings->volume) / (255 * 255);
    
    uint32_t pwm = ((mixed + 32768) >> (16 - settings->pwm.resolution));
    pwm = constrain(pwm, 0, (1 << settings->pwm.resolution) - 1);
    
    ledcWrite(settings->pwm.pin, pwm);
  }
  
  melodyPlayer.update();
}

// ============================================================================
// PLAYBACK CONTROL
// ============================================================================

int AudioEngine::findFreeVoice() {
  for (int i = 0; i < voiceCount; i++) {
    if (!voices[i].on) return i;
  }
  return 0;
}

void AudioEngine::noteOn(uint8_t note, uint8_t velocity) {
  int idx = findFreeVoice();
  voices[idx].noteOn(note, velocity, settings->sampleRate);
}

void AudioEngine::noteOff(uint8_t note) {
  for (int i = 0; i < voiceCount; i++) {
    if (voices[i].on && voices[i].note == note) {
      voices[i].noteOff();
    }
  }
}

void AudioEngine::allNotesOff() {
  for (int i = 0; i < voiceCount; i++) {
    voices[i].noteOff();
  }
}

void AudioEngine::playMelody(const Note* melody, size_t length) {
  melodyPlayer.play(melody, length);
}

void AudioEngine::stopMelody() {
  melodyPlayer.stop();
}

bool AudioEngine::isPlaying() {
  return melodyPlayer.isPlaying();
}

// ============================================================================
// SETTINGS: VOLUME
// ============================================================================

void AudioEngine::setVolume(uint8_t volume) {
  settings->volume = constrain(volume, 0, 255);
}

uint8_t AudioEngine::getVolume() {
  return settings->volume;
}

// ============================================================================
// SETTINGS: BIQUAD EQ
// ============================================================================

void AudioEngine::setEQ(int8_t bass, int8_t mid, int8_t treble) {
  settings->eq.bass = constrain(bass, -12, 12);
  settings->eq.mid = constrain(mid, -12, 12);
  settings->eq.treble = constrain(treble, -12, 12);
  updateEQFilters();
}

void AudioEngine::getEQ(int8_t& bass, int8_t& mid, int8_t& treble) {
  bass = settings->eq.bass;
  mid = settings->eq.mid;
  treble = settings->eq.treble;
}

void AudioEngine::setEQEnabled(bool enabled) {
  settings->eq.enabled = enabled;
  if (enabled) {
    updateEQFilters();
  }
}

bool AudioEngine::getEQEnabled() {
  return settings->eq.enabled;
}

// ============================================================================
// SETTINGS: STATE-VARIABLE FILTER
// ============================================================================

void AudioEngine::setFilterEnabled(bool enabled) {
  settings->filter.enabled = enabled;
  if (enabled) {
    updateFilterCoefficients();
  }
}

bool AudioEngine::getFilterEnabled() {
  return settings->filter.enabled;
}

void AudioEngine::setFilterType(FilterType type) {
  settings->filter.type = type;
}

FilterType AudioEngine::getFilterType() {
  return settings->filter.type;
}

const char* AudioEngine::getFilterTypeName() {
  return settings->filter.getTypeName();
}

void AudioEngine::setFilterCutoff(float cutoffHz) {
  settings->filter.cutoff = constrain(cutoffHz, FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX);
  updateFilterCoefficients();
}

float AudioEngine::getFilterCutoff() {
  return settings->filter.cutoff;
}

void AudioEngine::setFilterResonance(float resonance) {
  settings->filter.resonance = constrain(resonance, FILTER_RESONANCE_MIN, FILTER_RESONANCE_MAX);
  updateFilterCoefficients();
}

float AudioEngine::getFilterResonance() {
  return settings->filter.resonance;
}

// ============================================================================
// SETTINGS: SCHROEDER REVERB
// ============================================================================

void AudioEngine::setReverbEnabled(bool enabled) {
  if (enabled && !reverb.initialized) {
    if (!reverb.init()) {
      Serial.println(F("[ERROR] Failed to allocate reverb buffer"));
      settings->reverb.enabled = false;
      return;
    }
    updateReverbParameters();
  }
  
  settings->reverb.enabled = enabled;
  
  if (!enabled && reverb.initialized) {
    reverb.reset();
  }
}

bool AudioEngine::getReverbEnabled() {
  return settings->reverb.enabled;
}

void AudioEngine::setReverbRoomSize(float size) {
  settings->reverb.roomSize = constrain(size, 0.0f, 1.0f);
  updateReverbParameters();
}

float AudioEngine::getReverbRoomSize() {
  return settings->reverb.roomSize;
}

void AudioEngine::setReverbDamping(float damping) {
  settings->reverb.damping = constrain(damping, 0.0f, 1.0f);
}

float AudioEngine::getReverbDamping() {
  return settings->reverb.damping;
}

void AudioEngine::setReverbWet(float wet) {
  settings->reverb.wet = constrain(wet, 0.0f, 1.0f);
}

float AudioEngine::getReverbWet() {
  return settings->reverb.wet;
}

// ============================================================================
// SETTINGS: LFO (NEW!)
// ============================================================================

void AudioEngine::setLFOEnabled(bool enabled) {
  settings->lfo.enabled = enabled;
  if (enabled) {
    lfo.reset();
    updateLFORate();
  }
}

bool AudioEngine::getLFOEnabled() {
  return settings->lfo.enabled;
}

void AudioEngine::setLFOVibratoEnabled(bool enabled) {
  settings->lfo.vibratoEnabled = enabled;
}

bool AudioEngine::getLFOVibratoEnabled() {
  return settings->lfo.vibratoEnabled;
}

void AudioEngine::setLFOTremoloEnabled(bool enabled) {
  settings->lfo.tremoloEnabled = enabled;
}

bool AudioEngine::getLFOTremoloEnabled() {
  return settings->lfo.tremoloEnabled;
}

void AudioEngine::setLFORate(float rateHz) {
  settings->lfo.rate = constrain(rateHz, LFO_RATE_MIN, LFO_RATE_MAX);
  updateLFORate();
}

float AudioEngine::getLFORate() {
  return settings->lfo.rate;
}

void AudioEngine::setLFODepth(float depthPercent) {
  settings->lfo.depth = constrain(depthPercent, LFO_DEPTH_MIN, LFO_DEPTH_MAX);
}

float AudioEngine::getLFODepth() {
  return settings->lfo.depth;
}

// ============================================================================
// SETTINGS: DELAY
// ============================================================================

void AudioEngine::setDelayEnabled(bool enabled) {
  if (enabled && !delayBuffer) {
    if (!allocateDelayBuffer()) {
      Serial.println(F("[ERROR] Failed to allocate delay buffer"));
      settings->delay.enabled = false;
      return;
    }
  }
  
  settings->delay.enabled = enabled;
  
  if (enabled && delayBuffer) {
    memset(delayBuffer, 0, delayBufferSize * sizeof(int16_t));
    delayWritePos = 0;
  }
}

bool AudioEngine::getDelayEnabled() {
  return settings->delay.enabled;
}

void AudioEngine::setDelayTime(uint16_t ms) {
  settings->delay.timeMs = constrain(ms, 10, MAX_DELAY_TIME);
}

uint16_t AudioEngine::getDelayTime() {
  return settings->delay.timeMs;
}

void AudioEngine::setDelayFeedback(uint8_t percent) {
  settings->delay.feedback = constrain(percent, 0, 90);
}

uint8_t AudioEngine::getDelayFeedback() {
  return settings->delay.feedback;
}

void AudioEngine::setDelayMix(uint8_t percent) {
  settings->delay.mix = constrain(percent, 0, 100);
}

uint8_t AudioEngine::getDelayMix() {
  return settings->delay.mix;
}

// ============================================================================
// SETTINGS: WAVEFORM
// ============================================================================

void AudioEngine::setWaveform(WaveformType waveform) {
  settings->waveform = waveform;
  for (int i = 0; i < voiceCount; i++) {
    voices[i].waveform = waveform;
  }
}

WaveformType AudioEngine::getWaveform() {
  return settings->waveform;
}

const char* AudioEngine::getWaveformName() {
  switch(settings->waveform) {
    case WAVE_SINE:     return "Sine";
    case WAVE_SQUARE:   return "Square";
    case WAVE_SAWTOOTH: return "Sawtooth";
    case WAVE_TRIANGLE: return "Triangle";
    case WAVE_NOISE:    return "Noise";
    default:            return "Unknown";
  }
}

// ============================================================================
// STATUS
// ============================================================================

uint8_t AudioEngine::getActiveVoices() {
  uint8_t count = 0;
  for (int i = 0; i < voiceCount; i++) {
    if (voices[i].on) count++;
  }
  return count;
}
