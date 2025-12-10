# ESP32 Audio OS v1.0 - User Guide

**Version:** 1.0  
**Date:** 2025-11-28  
**Platform:** ESP32-C6 (Arduino)  
**Status:** Production Ready  

---

## 📋 Table of Contents

1. [Quick Start](#quick-start)
2. [Console Commands](#console-commands)
3. [Profile Management](#profile-management)
4. [Audio Effects](#audio-effects)
5. [Troubleshooting](#troubleshooting)
6. [FAQ](#faq)

---

## 🚀 Quick Start

### Hardware Setup

**Required Components:**
- ESP32-C6 Dev Board
- ST7789 Display (172x320, optional)
- 8002B Amplifier Module or Speaker
- USB Cable

**Wiring:**

| ESP32-C6 Pin | Component | Connection |
|--------------|-----------|------------|
| GPIO 1 | Audio Out | I2S DOUT (or Speaker via RC filter) |
| GPIO 6 | Display | MOSI |
| GPIO 7 | Display | SCLK |
| GPIO 14 | Display | CS |
| GPIO 15 | Display | DC |
| GPIO 21 | Display | RST |
| GPIO 22 | Display | BL |
| GND | Common | Ground |
| 5V | Power | 5V Supply |

### Software Installation

1. **Install Arduino IDE** (v2.x recommended)
2. **Add ESP32 Board Support:**
```

File → Preferences → Additional Board Manager URLs:
https://espressif.github.io/arduino-esp32/package_esp32_index.json

```
3. **Install Libraries:**
- ArduinoJson (v6.x or v7.x)
- Adafruit GFX Library
- Adafruit ST7789

4. **Flash Project:**
```

Tools → Board → ESP32C6 Dev Module
Tools → Upload Speed → 921600
Tools → Partition Scheme → Default 4MB with spiffs

```

5. **Upload SPIFFS Data:**
- Install [ESP32 Filesystem Uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin)
- Place `default.json` in `data/profiles/`
- Place `system.json` in `data/config/`
- Tools → ESP32 Sketch Data Upload

6. **Upload Sketch** → Click Upload Button

### First Boot

1. Open **Serial Monitor** (115200 baud)
2. You should see:
```

╔══════════════════════════════════════════════════════════════╗
║            ESP32 AUDIO OS v1.0                               ║
║            Tiny Audio Operating System                       ║
╚══════════════════════════════════════════════════════════════╝

[INFO] Target: ESP32-C6
[FS] ✓ Mounted: 1287 KB total
[AUDIO] Mode: I2S
[AUDIO] Sample Rate: 22050 Hz

[READY] Type 'audio help' for commands

audio>

```

3. **Test Audio:**
```

audio play

```
→ Should play Tetris melody!

---

## 🎮 Console Commands

### Playback Commands

#### `audio play [file]`
Play audio file or built-in melody.

**Examples:**
```

audio play              \# Play default (Tetris)
audio play tetris       \# Play Tetris explicitly
audio play test.wav     \# Play WAV file (future)

```

#### `audio stop`
Stop current playback.

```

audio stop

```

#### `audio volume <0-255>`
Set volume level.

```

audio volume 200        \# Default
audio volume 150        \# Quieter
audio volume 255        \# Maximum

```

#### `audio note <0-127>`
Play single MIDI note.

```

audio note 60           \# Middle C
audio note 69           \# A4 (440 Hz)

```

---

### Audio Effects

#### `audio eq <band> <value>`
Adjust equalizer.

**Bands:** `bass`, `mid`, `treble`  
**Range:** -12 to +12 dB

**Examples:**
```

audio eq bass 5         \# Boost bass +5 dB
audio eq treble -3      \# Reduce treble -3 dB
audio eq mid 0          \# Reset mid to flat

```

#### `audio reverb <amount>`
Set reverb amount.

**Range:** 0.0 to 1.0

```

audio reverb 0.3        \# Light reverb
audio reverb 0.0        \# Disable reverb

```

---

### Profile Management

#### `audio profile list`
List all saved profiles.

```

audio profile list

╔════════════════════════════════════════════════════════╗
║                 AVAILABLE PROFILES                     ║
╚════════════════════════════════════════════════════════╝

default [*]
gaming
music

[*] = currently active

```

#### `audio profile save <name>`
Save current settings as profile.

```

audio profile save gaming

```

#### `audio profile load <name>`
Load saved profile (requires restart).

```

audio profile load gaming
[WARN] Profile loaded - restart required
[HINT] Type 'audio reboot' to restart

```

#### `audio profile info <name>`
Show profile details.

```

audio profile info default

```

#### `audio profile delete <name>`
Delete profile (with confirmation).

```

audio profile delete old
[CONFIRM] Delete profile 'old'? (y/n): y

```

---

### System Commands

#### `audio info`
Show current configuration.

```

audio info

╔════════════════════════════════════════════════════════╗
║              CURRENT CONFIGURATION                     ║
╚════════════════════════════════════════════════════════╝

Profile:        default
Audio Mode:     I2S
Sample Rate:    22050 Hz
Voices:         4
Volume:         200/255
Pin:            GPIO 1 (I2S)

Effects:
EQ:           Bass +0, Mid +0, Treble +0 dB
Reverb:       0.00

Playing:        No
Active Voices:  0/4

```

#### `audio status`
Show system status.

```

audio status

╔════════════════════════════════════════════════════════╗
║                  SYSTEM STATUS                         ║
╚════════════════════════════════════════════════════════╝

Uptime:         00:15:42
CPU:            ESP32-C6 @ 160 MHz
Free RAM:       407 KB
Filesystem:     48 KB / 1287 KB used

Audio Engine:   I2S
Sample Rate:    22050 Hz
Playing:        No
Active Voices:  0/4

```

#### `audio version`
Show version information.

```

audio version

╔════════════════════════════════════════════════════════╗
║              ESP32 AUDIO OS                            ║
╚════════════════════════════════════════════════════════╝

Version:        1.0.0
Build Date:     2025-11-28
Target:         ESP32-C6
Cores:          Single

Features:
✓ Runtime configuration
✓ Profile system
✓ I2S + PWM support
✓ Smart resampling
✓ Codec plugins
✓ Full console control

```

#### `audio reboot`
Restart ESP32 (with confirmation).

```

audio reboot
[CONFIRM] Reboot ESP32? (y/n): y
[REBOOT] Restarting in 2 seconds...

```

---

### Hardware Configuration

#### `audio mode <i2s|pwm>`
Switch audio mode (requires restart).

```

audio mode pwm
[OK] Mode changed to PWM
[WARN] Restart required - type 'audio reboot'

```

#### `audio hw show`
Show hardware configuration.

```

audio hw show

╔════════════════════════════════════════════════════════╗
║              HARDWARE CONFIGURATION                    ║
╚════════════════════════════════════════════════════════╝

Current Mode:   I2S

I2S Settings:
Pin:          GPIO 1
Buffer Size:  64 samples
Buffers:      2
Amplitude:    12000

```

#### `audio hw pin <1-48>`
Change audio output pin (requires restart).

```

audio hw pin 2
[OK] Pin set to GPIO 2
[WARN] Restart required

```

---

### Codec Management

#### `audio codec list`
List available codecs.

```

audio codec list

╔════════════════════════════════════════════════════════╗
║                    AVAILABLE CODECS                    ║
╚════════════════════════════════════════════════════════╝

NAME    VERSION   STATUS      MEMORY    CPU     FORMATS
────────────────────────────────────────────────────────
WAV     1.0       Built-in    8 KB      5%      .wav

```

#### `audio codec info <name>`
Show codec details.

```

audio codec info wav

```

---

## 📁 Profile Management

### Profile Structure

Profiles are stored in `/profiles/<name>.json` on SPIFFS.

**Example Profile:**
```

{
"name": "gaming",
"description": "Optimized for game audio",
"audio": {
"mode": "i2s",
"sampleRate": 22050,
"voices": 4,
"volume": 220
},
"effects": {
"eq": {
"bass": 5,
"mid": 0,
"treble": 3
},
"reverb": 0.2
}
}

```

### Creating Custom Profiles

1. **Adjust Settings:**
```

audio volume 220
audio eq bass 5
audio eq treble 3
audio reverb 0.2

```

2. **Save Profile:**
```

audio profile save gaming
[OK] Profile 'gaming' saved

```

3. **Use Profile:**
```

audio profile load gaming
audio reboot

```

### Default Profile

The `default` profile is loaded on startup. You can change this:

```

audio config startup gaming
[OK] Startup profile set to 'gaming'

```

---

## 🎵 Audio Effects

### Equalizer (EQ)

**3-Band Equalizer:**
- **Bass:** 60-250 Hz
- **Mid:** 250-4000 Hz
- **Treble:** 4000-16000 Hz

**Range:** -12 to +12 dB

**Common Presets:**

```


# Flat (default)

audio eq bass 0
audio eq mid 0
audio eq treble 0

# Boost Bass

audio eq bass 8
audio eq mid 0
audio eq treble 0

# Vocal Enhancement

audio eq bass 0
audio eq mid 5
audio eq treble 3

# Bright

audio eq bass -3
audio eq mid 0
audio eq treble 5

```

### Reverb

**Range:** 0.0 (off) to 1.0 (maximum)

**Typical Values:**
- `0.0` - No reverb (dry)
- `0.2` - Light room
- `0.4` - Medium hall
- `0.6` - Large hall
- `1.0` - Cathedral

**Example:**
```

audio reverb 0.3
[OK] Reverb: 0.30

```

---

## 🔧 Troubleshooting

### No Audio Output

**Symptoms:** No sound from speaker

**Checks:**
1. Serial Monitor shows `[I2S] ✓`?
2. GPIO 1 connected to amplifier?
3. Amplifier has power (5V)?
4. Speaker connected?
5. Volume not zero?

**Test:**
```

audio volume 200
audio play

```

**Advanced Test:**
```

audio test 440 1000    \# Play 440 Hz for 1000 ms

```

---

### Distorted Audio

**Symptoms:** Crackling, clipping, noise

**Solutions:**

1. **Reduce Volume:**
```

audio volume 150

```

2. **Check EQ Settings:**
```

audio eq bass 0
audio eq mid 0
audio eq treble 0

```

3. **Add RC Filter:** (if using direct GPIO output)
- GPIO 1 → 1kΩ resistor → Speaker
- 100nF capacitor between resistor & GND

---

### Commands Not Working

**Symptoms:** Typing commands shows no response

**Checks:**
1. Serial Monitor baud rate = 115200?
2. Line ending = "Newline" or "Both NL & CR"?
3. Sketch uploaded successfully?

**Test:**
```

audio help     \# Should show command list

```

---

### SPIFFS Mount Failed

**Symptoms:** `[ERROR] Filesystem init failed!`

**Solution:**
1. Upload SPIFFS data:
   - Tools → ESP32 Sketch Data Upload
2. Check partition scheme:
   - Tools → Partition Scheme → "Default 4MB with spiffs"
3. If still fails, format:
```

// In code, temporarily:
SPIFFS.format();

```

---

### Profile Not Loading

**Symptoms:** Settings not applied after `audio profile load`

**Cause:** Profile changes require restart

**Solution:**
```

audio profile load gaming
audio reboot

```

---

## ❓ FAQ

### Can I use PWM instead of I2S?

**Yes!** PWM mode uses only GPIO pin, no external DAC needed.

```

audio mode pwm
audio reboot

```

**Quality:**
- **I2S:** 16-bit, ~96 dB SNR, perfect
- **PWM:** 9-bit, ~80 dB SNR, good enough for beeps/melodies

---

### How do I add custom melodies?

Edit the `melodyTetris` array in `AudioConsole.cpp`:

```

static const Note myMelody[] = {
{NOTE_C5, 500, 255},  // C5, 500ms, full velocity
{NOTE_D5, 500, 200},
{NOTE_E5, 1000, 255}
};

```

Then change play command to use it.

---

### Can I play WAV files?

**Future feature!** Currently only built-in melodies supported.

WAV playback infrastructure is ready (see `AudioCodec_WAV`), but file streaming not yet implemented.

---

### What sample rates are supported?

**Default:** 22050 Hz (optimized for ESP32-C6)

**Range:** 8000-48000 Hz (configurable in profile)

**Recommendation:** Stick with 22050 Hz unless you have a specific need.

---

### How many voices can play simultaneously?

**Default:** 4 voices

**Maximum:** 8 voices (requires more CPU)

Change in `AudioConfig.h`:
```

\#define DEFAULT_MAX_VOICES 8

```

---

### Does it work on ESP32 Classic or ESP32-S3?

**Yes, with modifications:**
- ESP32 Classic: Works, has dual core advantage
- ESP32-S3: Works, has dual core + LP core
- ESP32-C3: Works, single core like C6

Main differences:
- I2S initialization (already handled in code)
- Pin assignments
- Some optimization opportunities (dual core)

---

### Can I control it via WiFi/Bluetooth?

**Not yet!** Currently serial-only.

Future roadmap includes:
- Web interface (WiFi)
- Bluetooth audio streaming
- MQTT control

---

### How much RAM/Flash does it use?

**Flash:** ~457 KB (34% of 1310 KB)  
**RAM:** ~16 KB (5% of 327 KB)  

Plenty of room for expansion!

---

## 📚 Further Reading

- **Technical Documentation:** See `ESP32_AudioOS_v1.0_TechnicalDoc.md`
- **Hardware Details:** See `ESP32_AudioOS_v1.0_HardwareGuide.md`
- **Research Notes:** See `ESP32_AudioOS_v1.0_ResearchNotes.md`

---

## 🎉 You're Ready!

The Audio OS is production-ready. Experiment, customize, and enjoy!

**Quick Recap:**
```

audio play          \# Play melody
audio volume 200    \# Adjust volume
audio eq bass 5     \# Boost bass
audio profile save  \# Save settings
audio help          \# Show all commands

```

**Have fun!** 🎵

---

*Document Version: 1.0*  
*Last Updated: 2025-11-28*  
*For: ESP32 Audio OS v1.0*
```


