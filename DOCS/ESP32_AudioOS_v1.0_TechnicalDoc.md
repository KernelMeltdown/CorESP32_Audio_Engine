# ESP32 Audio OS v1.0 - Technical Documentation

**Version:** 1.0  
**Date:** 2025-11-28  
**Platform:** ESP32-C6 (Arduino Core 3.x, ESP-IDF 5.x)  
**Architecture:** Single-core RISC-V @ 160 MHz  

---

## 📋 Table of Contents

1. [System Architecture](#system-architecture)
2. [Audio Engine](#audio-engine)
3. [Codec System](#codec-system)
4. [Profile System](#profile-system)
5. [Console Interface](#console-interface)
6. [Filesystem](#filesystem)
7. [API Reference](#api-reference)

---

## 🏗️ System Architecture

### High-Level Overview

```

┌─────────────────────────────────────────────────────────┐
│                    Application Layer                    │
│              (ESP32_UniversalAudio.ino)                 │
└─────────────────────────────────────────────────────────┘
│
┌──────────────────┼──────────────────┐
│                  │                  │
▼                  ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   Console    │  │ Audio Engine │  │  Profile     │
│              │  │              │  │  Manager     │
└──────────────┘  └──────────────┘  └──────────────┘
│                  │                  │
└──────────────────┼──────────────────┘
│
┌──────────────────┼──────────────────┐
│                  │                  │
▼                  ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│  Filesystem  │  │ Codec Manager│  │  Hardware    │
│   (SPIFFS)   │  │              │  │  (I2S/PWM)   │
└──────────────┘  └──────────────┘  └──────────────┘

```

### Component Responsibilities

| Component | Responsibility | File(s) |
|-----------|----------------|---------|
| **Audio Engine** | Sample generation, mixing, output | `AudioEngine.cpp/h` |
| **Console** | Command parsing, user interface | `AudioConsole.cpp/h` |
| **Profile Manager** | Settings persistence, loading | `AudioProfile.cpp/h` |
| **Filesystem** | SPIFFS wrapper, file operations | `AudioFilesystem.cpp/h` |
| **Codec Manager** | Plugin loading, codec selection | `AudioCodecManager.cpp/h` |
| **Settings** | Runtime configuration container | `AudioSettings.h` |
| **Resampler** | Sample rate conversion | `AudioResampler.cpp/h` |

---

## 🎵 Audio Engine

### Architecture

```

Audio Engine (AudioEngine.cpp)
├─ Voice Management (4 polyphonic voices)
│  ├─ MIDI Note Tracking
│  ├─ ADSR Envelope Generation
│  └─ Sample-accurate rendering
│
├─ Audio Output Modes
│  ├─ I2S (DMA-driven, FreeRTOS task)
│  └─ PWM (Loop-based, ledcWrite)
│
└─ Effects Chain
├─ Voice Mixing
├─ EQ (3-band)
├─ Reverb
└─ Master Volume

```

### I2S Mode (Primary)

**How it works:**

1. **FreeRTOS Task** runs at highest priority
2. **DMA Controller** handles timing automatically
3. **Double Buffering:** Task fills buffer while DMA outputs previous one
4. **Sample Rate:** Hardware-accurate 22050 Hz

**Code Flow:**
```

void audioTask(void* parameter) {
int16_t buffer[I2S_BUFFER_SIZE * 2]; // Stereo

while(1) {
// Fill buffer
for (int i = 0; i < I2S_BUFFER_SIZE; i++) {
int32_t mixed = mixVoices();
buffer[i*2] = buffer[i*2+1] = (int16_t)mixed; // Mono to Stereo
}

    // Write to I2S (blocks until DMA ready)
    size_t written;
    i2s_channel_write(tx_handle, buffer, sizeof(buffer), &written, portMAX_DELAY);
    }
}

```

**Advantages:**
- ✅ Perfect timing (crystal-accurate)
- ✅ Low CPU usage (~5%)
- ✅ No jitter
- ✅ 16-bit output

**Configuration:**
```

i2s_chan_config_t chan_cfg = {
.id = I2S_NUM_0,
.role = I2S_ROLE_MASTER,
.dma_desc_num = 2,
.dma_frame_num = 64,
.auto_clear = true
};

i2s_std_config_t std_cfg = {
.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(22050),
.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
I2S_DATA_BIT_WIDTH_16BIT,
I2S_SLOT_MODE_STEREO
),
.gpio_cfg = {
.dout = (gpio_num_t)settings->i2s.pin,
// ... other pins unused
}
};

```

---

### PWM Mode (Fallback)

**How it works:**

1. **Main Loop** checks timing with `micros()`
2. **Every 45µs** (22050 Hz), generate one sample
3. **LEDC Hardware** converts duty cycle to PWM waveform
4. **External Filter** smooths PWM to analog

**Code Flow:**
```

void loop() {
static uint32_t lastMicros = 0;
uint32_t now = micros();

if (now - lastMicros >= 45) {  // 1/22050 Hz = 45.35µs
lastMicros = now;

    int32_t mixed = mixVoices();
    
    // Convert -32768..32767 to 0..511 (9-bit PWM)
    uint32_t pwm = ((mixed + 32768) >> 7);
    ledcWrite(AUDIO_PIN, pwm);
    }

// Other tasks (console, display, etc.)
}

```

**Advantages:**
- ✅ Simple, no external DAC
- ✅ Works on any GPIO
- ✅ Predictable behavior

**Disadvantages:**
- ⚠️ Timing jitter from interrupts
- ⚠️ 9-bit resolution (vs 16-bit I2S)
- ⚠️ Higher CPU usage (~15%)

**Configuration:**
```

\#define PWM_FREQUENCY   78125   // Hz
\#define PWM_RESOLUTION  9       // bits (512 levels)
\#define PWM_AMPLITUDE   5000    // 16-bit scaled
\#define PWM_GAIN        7       // Post-mix boost

ledcAttach(pin, PWM_FREQUENCY, PWM_RESOLUTION);

```

---

### Voice Structure

Each voice is independent and manages its own state:

```

struct Voice {
// State
bool active;
uint8_t note;        // MIDI note (0-127)
uint8_t velocity;    // Velocity (0-127)

// Oscillator
float phase;         // Current phase (0.0-1.0)
float phaseInc;      // Phase increment per sample

// Envelope (ADSR)
enum EnvState { OFF, ATTACK, DECAY, SUSTAIN, RELEASE };
EnvState envState;
uint32_t envSampleCount;

// Methods
void noteOn(uint8_t note, uint8_t velocity);
void noteOff();
int16_t getSample();  // Render one sample
};

```

**ADSR Envelope Parameters:**
```

\#define ENV_ATTACK_SAMPLES   441   // ~20ms @ 22050 Hz
\#define ENV_DECAY_SAMPLES    882   // ~40ms
\#define ENV_RELEASE_SAMPLES  1764  // ~80ms
\#define ENV_SUSTAIN_LEVEL    200   // 0-255

```

**Sample Generation:**
```

int16_t Voice::getSample() {
if (!active) return 0;

// 1. Generate waveform (sine, saw, square, triangle)
float sample = sin(phase * 2.0f * PI);

// 2. Advance phase
phase += phaseInc;
if (phase >= 1.0f) phase -= 1.0f;

// 3. Apply envelope
uint8_t envLevel = getEnvelopeLevel();
sample *= (envLevel / 255.0f);

// 4. Apply velocity
sample *= (velocity / 127.0f);

// 5. Scale to 16-bit
return (int16_t)(sample * 32767.0f);
}

```

---

### Melody System

**Data Structure:**
```

struct Note {
uint8_t pitch;      // MIDI note or NOTE_REST (0)
uint16_t duration;  // Milliseconds
uint8_t velocity;   // 0-255
};

const Note melodyTetris[] = {
{NOTE_E5, 400, 255},
{NOTE_B4, 200, 255},
{NOTE_C5, 200, 255},
// ...
};

```

**Playback:**
```

class MelodyPlayer {
const Note* melody;
size_t melodyLen;
size_t currentNote;
uint32_t noteStartTime;
bool playing;

void play(const Note* m, size_t len);
void stop();
void update();  // Call in loop()
};

```

**Update Logic:**
```

void MelodyPlayer::update() {
if (!playing) return;

if (millis() - noteStartTime >= melody[currentNote].duration) {
// Stop current note
if (melody[currentNote].pitch != NOTE_REST) {
audio->noteOff(melody[currentNote].pitch);
}

    // Advance to next note
    currentNote++;
    if (currentNote >= melodyLen) {
      playing = false;
      return;
    }
    
    // Start new note
    noteStartTime = millis();
    if (melody[currentNote].pitch != NOTE_REST) {
      audio->noteOn(melody[currentNote].pitch, melody[currentNote].velocity);
    }
    }
}

```

---

## 🎛️ Codec System

### Plugin Architecture

```

Codec Manager (AudioCodecManager)
├─ Codec Registry (list of available codecs)
│  ├─ WAV (built-in)
│  ├─ MP3 (future)
│  └─ AAC (future)
│
├─ Auto-Detection (by file extension)
└─ Codec Interface (abstract base class)

```

**Abstract Interface:**
```

class AudioCodec {
public:
virtual bool canDecode(const char* filename) = 0;
virtual bool open(const char* filename) = 0;
virtual void close() = 0;
virtual int16_t readSample() = 0;
virtual AudioFormat getFormat() = 0;
virtual bool isEOF() = 0;
};

```

**Format Descriptor:**
```

struct AudioFormat {
uint32_t sampleRate;  // Hz
uint8_t bitDepth;     // 8, 16, 24, 32
uint8_t channels;     // 1 (mono), 2 (stereo)
const char* codecName;
};

```

---

### WAV Codec Implementation

**Header Parsing:**
```

struct WavHeader {
char riff;           // "RIFF"
uint32_t fileSize;
char wave;           // "WAVE"
char fmt;            // "fmt "
uint32_t fmtSize;
uint16_t audioFormat;   // 1 = PCM
uint16_t numChannels;
uint32_t sampleRate;
uint32_t byteRate;
uint16_t blockAlign;
uint16_t bitsPerSample;
char data;           // "data"
uint32_t dataSize;
};

```

**Validation:**
```

bool AudioCodec_WAV::validate() {
if (strncmp(header.riff, "RIFF", 4) != 0) return false;
if (strncmp(header.wave, "WAVE", 4) != 0) return false;
if (header.audioFormat != 1) return false;  // Only PCM
if (header.bitsPerSample != 8 \&\& header.bitsPerSample != 16) return false;
return true;
}

```

**Sample Reading:**
```

int16_t AudioCodec_WAV::readSample() {
if (header.bitsPerSample == 8) {
uint8_t sample8 = readByte();
return (int16_t)((sample8 - 128) << 8);  // Unsigned to signed
} else {
int16_t sample16 = readInt16();
return sample16;
}
}

```

**Auto-Detection:**
```

AudioCodec* AudioCodecManager::detectCodec(const char* filename) {
for (AudioCodec* codec : codecs) {
if (codec->canDecode(filename)) {
return codec;
}
}
return nullptr;
}

```

---

## 📦 Profile System

### JSON Schema

**Profile Format:**
```

{
"schema_version": "1.0",
"engine_version": "1.0.0",
"name": "default",
"description": "Factory default audio settings",

"audio": {
"mode": "i2s",
"sampleRate": 22050,
"voices": 4,
"volume": 200
},

"hardware": {
"i2s": {
"pin": 1,
"bufferSize": 64,
"numBuffers": 2,
"amplitude": 12000
},
"pwm": {
"pin": 2,
"frequency": 78125,
"resolution": 9,
"amplitude": 5000,
"gain": 7
}
},

"effects": {
"eq": {
"bass": 0,
"mid": 0,
"treble": 0
},
"reverb": 0.0
},

"resample": {
"quality": "best"
},

"display": {
"enabled": true,
"brightness": 180
}
}

```

---

### Profile Loading Process

```

Boot
│
├─> Load system.json
│    ├─> Get startup profile name
│    └─> Load profile (e.g., "default")
│
├─> Parse JSON with ArduinoJson
│
├─> Validate schema version
│
├─> Populate AudioSettings struct
│
└─> Pass to AudioEngine::init()

```

**Code:**
```

bool AudioProfile::loadProfile(const char* name) {
char path;[^1]
snprintf(path, sizeof(path), "/profiles/%s.json", name);

File file = filesystem->open(path, "r");
if (!file) return false;

StaticJsonDocument<2048> doc;
DeserializationError error = deserializeJson(doc, file);
if (error) return false;

// Validate schema
if (strcmp(doc["schema_version"], PROFILE_SCHEMA_VERSION) != 0) {
return false;
}

// Populate settings
settings.sampleRate = doc["audio"]["sampleRate"];
settings.volume = doc["audio"]["volume"];
// ... etc

return true;
}

```

---

### Profile Saving

```

bool AudioProfile::saveProfile(const char* name) {
char path;[^1]
snprintf(path, sizeof(path), "/profiles/%s.json", name);

StaticJsonDocument<2048> doc;

// Serialize current settings
doc["schema_version"] = PROFILE_SCHEMA_VERSION;
doc["engine_version"] = AUDIO_OS_VERSION;
doc["name"] = name;

doc["audio"]["mode"] = settings.getModeName();
doc["audio"]["sampleRate"] = settings.sampleRate;
doc["audio"]["voices"] = settings.voices;
doc["audio"]["volume"] = settings.volume;

// ... serialize all settings

File file = filesystem->open(path, "w");
if (!file) return false;

serializeJson(doc, file);
file.close();

return true;
}

```

---

## 🖥️ Console Interface

### Command Parser

**Input Processing:**
```

void AudioConsole::update() {
while (Serial.available()) {
char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (cmdBuffer.length() > 0) {
        processCommand(cmdBuffer);
        cmdBuffer = "";
        printPrompt();
      }
    } else if (c == 8 || c == 127) {  // Backspace
      if (cmdBuffer.length() > 0) {
        cmdBuffer.remove(cmdBuffer.length() - 1);
        Serial.print(F("\b \b"));
      }
    } else if (c >= 32 && c < 127) {  // Printable
      cmdBuffer += c;
      Serial.print(c);
    }
    }
}

```

**Command Routing:**
```

void AudioConsole::processCommand(String cmd) {
String subCmd = getArg(cmd, 0);
subCmd.toLowerCase();

// Handle "audio" prefix
if (subCmd == "audio") {
subCmd = getArg(cmd, 1);
// Remove prefix from args
}

// Route to handlers
if (subCmd == "play") cmdPlay(cmd);
else if (subCmd == "stop") cmdStop(cmd);
else if (subCmd == "volume") cmdVolume(cmd);
// ... etc
}

```

---

## 📂 Filesystem

### SPIFFS Layout

```

/
├── config/
│   ├── system.json          \# System configuration
│   └── codec_registry.json  \# Codec metadata (future)
│
├── profiles/
│   ├── default.json         \# Default profile
│   ├── gaming.json          \# User profiles
│   └── music.json
│
├── audio/                   \# Audio files (future)
│   ├── test.wav
│   └── samples/
│
└── melodies/                \# MIDI melody files (future)
└── tetris.mid

```

**Total:** 1287 KB available, 48 KB used initially

---

### Filesystem Wrapper

**Purpose:** Simplify SPIFFS operations

```

class AudioFilesystem {
public:
bool init();
bool isInitialized();

File open(const char* path, const char* mode);
bool exists(const char* path);
bool remove(const char* path);
void listDir(const char* path);

size_t totalBytes();
size_t usedBytes();
};

```

**Initialization:**
```

bool AudioFilesystem::init() {
if (!SPIFFS.begin(false)) {  // false = don't format on fail
Serial.println(F("[ERROR] SPIFFS mount failed!"));
return false;
}

initialized = true;

Serial.printf("[FS] ✓ Mounted: %d KB total, %d KB used\n",
SPIFFS.totalBytes() / 1024,
SPIFFS.usedBytes() / 1024);

return true;
}

```

---

## 🔧 API Reference

### AudioEngine

```

class AudioEngine {
public:
// Initialization
bool init(AudioSettings* settings);
void deinit();
void update();  // Call in loop() for PWM mode

// Playback Control
void noteOn(uint8_t note, uint8_t velocity = 127);
void noteOff(uint8_t note);
void allNotesOff();
void playMelody(const Note* melody, size_t length);
void stopMelody();

// Effects
void setVolume(uint8_t level);
uint8_t getVolume();
void setEQ(int8_t bass, int8_t mid, int8_t treble);
void getEQ(int8_t\& bass, int8_t\& mid, int8_t\& treble);
void setReverb(float amount);
float getReverb();

// Status
bool isPlaying();
uint8_t getActiveVoices();
uint8_t getVoiceCount();
uint32_t getSampleRate();
const char* getModeName();
AudioSettings* getSettings();
};

```

---

### AudioProfile

```

class AudioProfile {
public:
void init(AudioFilesystem* fs);

bool loadProfile(const char* name);
bool saveProfile(const char* name);
bool deleteProfile(const char* name);
void listProfiles();
void showProfileInfo(const char* name);

bool loadStartupProfile();
void setStartupProfile(const char* name);

void createDefaultProfile();
bool validateProfile(const char* name);

AudioSettings* getCurrentSettings();
};

```

---

### AudioCodecManager

```

class AudioCodecManager {
public:
void init(AudioFilesystem* fs);
void registerCodec(AudioCodec* codec);
AudioCodec* detectCodec(const char* filename);
bool canDecode(const char* codecName, const char* filename);

void listCodecs();
void showCodecInfo(const char* name);
};

```

---

## 🧪 Testing & Validation

### Unit Tests

**Test Audio Engine:**
```

void testAudioEngine() {
AudioEngine engine;
AudioSettings settings;

assert(engine.init(\&settings));
assert(engine.getVoiceCount() == 4);

engine.noteOn(60, 127);  // Middle C
assert(engine.getActiveVoices() == 1);

engine.noteOff(60);
assert(engine.getActiveVoices() == 0);
}

```

**Test Profile System:**
```

void testProfiles() {
AudioProfile profile;
AudioFilesystem fs;

fs.init();
profile.init(\&fs);

assert(profile.loadProfile("default"));

AudioSettings* settings = profile.getCurrentSettings();
assert(settings->sampleRate == 22050);
}

```

---

## 📊 Performance Metrics

### Measured Performance (ESP32-C6 @ 160 MHz)

| Metric | I2S Mode | PWM Mode |
|--------|----------|----------|
| CPU Usage (idle) | 2% | 5% |
| CPU Usage (4 voices) | 7% | 18% |
| Audio Latency | 12 ms | 15 ms |
| Sample Rate Jitter | < 1 µs | 5-10 µs |
| Flash Usage | 457 KB | 457 KB |
| RAM Usage | 16 KB | 14 KB |

### Optimization Tips

1. **Use I2S:** Lower CPU, better quality
2. **Reduce Voices:** 2 voices = 50% less CPU
3. **Lower Sample Rate:** 11025 Hz = 50% less CPU (lower quality)
4. **Disable Effects:** EQ/Reverb add ~3% CPU each
5. **Optimize Loop:** Keep `loop()` fast for PWM mode

---

## 🔐 Error Handling

### Initialization Failures

```

if (!filesystem.init()) {
Serial.println(F("[WARN] Filesystem unavailable - profiles won't persist"));
// Continue without SPIFFS
}

if (!audio.init(settings)) {
Serial.println(F("[FATAL] Audio engine init failed!"));
while(1) delay(1000);  // Halt system
}

```

### Runtime Errors

```

// Graceful degradation
if (codec->open(filename)) {
// Play file
} else {
Serial.println(F("[ERROR] Failed to open file"));
// Fall back to built-in melody
audio.playMelody(melodyTetris, sizeof(melodyTetris) / sizeof(Note));
}

```

---

## 🚀 Future Extensions

### Planned Features

1. **WAV File Playback**
   - Stream from SPIFFS
   - Real-time sample rate conversion
   - Stereo → Mono downmix

2. **MP3 Codec**
   - minimp3 library integration
   - 10x storage savings vs WAV

3. **SD Card Support**
   - Large audio libraries
   - Gapless playback

4. **WiFi/Bluetooth**
   - Web-based control interface
   - Bluetooth audio streaming

5. **Advanced Synthesis**
   - FM synthesis
   - Wavetable morphing
   - MIDI controller input

---

## 📚 References

- **ESP32-C6 Technical Reference Manual:** [espressif.com](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- **I2S Protocol:** [NXP I2S Specification](https://www.nxp.com/docs/en/user-guide/UM10732.pdf)
- **ArduinoJson Documentation:** [arduinojson.org](https://arduinojson.org/)
- **ADSR Envelope Theory:** [Wikipedia](https://en.wikipedia.org/wiki/Envelope_(music))

---

*Document Version: 1.0*  
*Last Updated: 2025-11-28*  
*For: ESP32 Audio OS v1.0*
```


***
