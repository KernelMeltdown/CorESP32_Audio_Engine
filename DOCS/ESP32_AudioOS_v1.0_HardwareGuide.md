# ESP32 Audio OS v1.0 - Hardware Guide

**Version:** 1.0  
**Date:** 2025-11-28  
**Platform:** ESP32-C6 Development Board  
**Status:** Production Reference  

---

## 📋 Table of Contents

1. [ESP32-C6 Specifications](#esp32-c6-specifications)
2. [Pin Configuration](#pin-configuration)
3. [Audio Output Options](#audio-output-options)
4. [Display Integration](#display-integration)
5. [Power Requirements](#power-requirements)
6. [Troubleshooting Hardware Issues](#troubleshooting-hardware-issues)

---

## 🔌 ESP32-C6 Specifications

### Core Specifications

| Feature | Specification |
|---------|---------------|
| **CPU** | RISC-V 32-bit, single-core |
| **Clock Speed** | 160 MHz (default), up to 240 MHz |
| **RAM** | 512 KB SRAM (HP: 496 KB, LP: 16 KB) |
| **Flash** | 4 MB (onboard SPI) |
| **GPIO Pins** | 26 available (some shared) |
| **WiFi** | 802.11 b/g/n (2.4 GHz) |
| **Bluetooth** | BLE 5.0 |
| **USB** | USB 2.0 Full Speed (built-in) |
| **Operating Voltage** | 3.3V |
| **Input Voltage** | 5V (via USB or Vin) |

### Peripherals Used by Audio OS

| Peripheral | Usage | Pin(s) |
|------------|-------|--------|
| **LEDC** | PWM Audio Output | GPIO 1 (default) |
| **I2S** | Digital Audio Output | GPIO 1 (DOUT) |
| **SPI** | Display Communication | GPIO 6, 7 |
| **UART0** | Serial Console | GPIO 16 (TX), 17 (RX) |
| **GPIO** | Display Control | GPIO 14, 15, 21, 22 |

### Memory Map

```

Flash (4 MB):
├─ 0x000000 - 0x00FFFF   Bootloader (64 KB)
├─ 0x010000 - 0x10FFFF   App Partition (1 MB)
├─ 0x110000 - 0x3FFFFF   SPIFFS (896 KB)
└─ 0x009000 - 0x00FFFF   NVS + PHY Data (40 KB)

RAM (512 KB):
├─ 0x40800000           SRAM0 (HP)
├─ 0x50000000           SRAM1 (LP, 16 KB)
└─ Stack/Heap           Dynamic allocation

```

---

## 📌 Pin Configuration

### Default Pin Assignment

```

ESP32-C6 DevKit Pinout:

● 3.3V
● RST
● GPIO 1
● GPIO 3
● GPIO 5
● (SCLK) GPIO 7
● GPIO 9 
● GPIO 11
● GPIO 13
● (TFT DC) GPIO 15
● (UART RX) GPIO 17
● GPIO 19
● (TFT RST) GPIO 21 
● GPIO 23           
● GND
● GPIO 0
● GPIO 2
● GPIO 4
● GPIO 6  (MOSI)
● GPIO 8
● GPIO 10
● GPIO 12
● GPIO 14 (TFT CS)
● GPIO 16 (UART TX)
● GPIO 18
● GPIO 20
● GPIO 22 (TFT BL)
● 5V
●3.3V



### Pin Usage Table

| GPIO | Function | Direction | Note |
|------|----------|-----------|------|
| 0 | Boot Mode | Input | Hold LOW for flash mode |
| 1 | Audio Out (I2S/PWM) | Output | **Primary audio pin** |
| 2 | User LED / PWM Alt | Output | Alternative audio pin |
| 6 | Display MOSI | Output | SPI data |
| 7 | Display SCLK | Output | SPI clock |
| 14 | Display CS | Output | Chip select |
| 15 | Display DC | Output | Data/Command |
| 16 | UART TX | Output | Serial console |
| 17 | UART RX | Input | Serial console |
| 21 | Display RST | Output | Reset |
| 22 | Display BL | Output | Backlight (PWM) |

### Pin Constraints

**Strapping Pins (Boot Mode):**
- GPIO 8, 9: Affect boot mode - avoid for critical functions
- GPIO 15: Pull-up at boot

**ADC Capable:**
- GPIO 0-7: Can be used for analog input (not used in Audio OS)

**Safe for General Use:**
- GPIO 1-7, 10-23: No boot conflicts

---

## 🔊 Audio Output Options

### Option 1: I2S Digital Output (Recommended)

**Hardware Setup:**
```

ESP32-C6 GPIO 1 (I2S DOUT)
│
└──> I2S DAC Module (e.g., PCM5102A, UDA1334A)
│
└──> Amplifier (e.g., PAM8403)
│
└──> Speaker (8Ω)

```

**I2S DAC Modules:**

| Module | Price | Bit Depth | SNR | Notes |
|--------|-------|-----------|-----|-------|
| **PCM5102A** | ~$5 | 24-bit | 112 dB | Best quality, requires 3.3V |
| **UDA1334A** | ~$8 | 24-bit | 100 dB | Adafruit breakout, easy to use |
| **MAX98357A** | ~$7 | 16-bit | 92 dB | Built-in 3W amplifier |

**Wiring Example (PCM5102A):**
```

ESP32-C6          PCM5102A
─────────────────────────
GPIO 1 (DOUT) ──> DIN
GND           ──> GND
3.3V          ──> VCC
──> SCK (tied to GND for I2S mode)
──> BCK (tied to GND)
──> LRCK (tied to GND)

PCM5102A          Amplifier/Speaker
───────────────────────────────────
OUTR          ──> Right Input
OUTL          ──> Left Input

```

**Advantages:**
- ✅ Perfect audio quality (16-24 bit)
- ✅ No noise or distortion
- ✅ Low CPU usage (DMA-driven)
- ✅ Professional sound

**Disadvantages:**
- ⚠️ Requires external DAC module
- ⚠️ More complex wiring
- ⚠️ Slightly higher cost

---

### Option 2: PWM Direct Output (Simple)

**Hardware Setup:**
```

ESP32-C6 GPIO 1 (PWM)
│
├── 1kΩ Resistor
│       │
│   ┌───┴───┐
│   │ 100nF │
│   │ Cap   │
│   └───────┘
│       │
│      GND
│
└──> Speaker (8Ω) or Amplifier Input

```

**RC Low-Pass Filter:**
- **R = 1 kΩ**
- **C = 100 nF**
- **Cutoff Frequency:** fc = 1/(2πRC) ≈ 1.6 kHz

**Purpose:** Smooth PWM carrier (78 kHz) to analog audio signal.

**Wiring Diagram:**
```

┌──────────────┐
│  ESP32-C6    │
│              │
│  GPIO 1  ────┼──── 1kΩ ────┬──── To Speaker/Amp
│              │              │
│  GND     ────┼──────────────┼──── GND
└──────────────┘              │
100nF
│
GND

```

**Advantages:**
- ✅ Minimal components (2 parts)
- ✅ Direct connection to GPIO
- ✅ Low cost (<$0.50)
- ✅ Good enough for melodies/beeps

**Disadvantages:**
- ⚠️ Lower quality than I2S (9-bit vs 16-bit)
- ⚠️ Some noise/hiss
- ⚠️ Limited dynamic range

---

### Option 3: 8002B Amplifier Module (Used in Development)

**Hardware Setup:**
```

ESP32-C6          8002B Module          Speaker
─────────────────────────────────────────────
GPIO 1 (PWM)  ──> S (Signal)
GND           ──> G (Ground)
5V            ──> V (Power, 5V)
+ ──────> Speaker (+)
- ──────> Speaker (-)

```

**8002B Specifications:**
- **Power:** 2W @ 8Ω, 3W @ 4Ω
- **Voltage:** 2.0V - 5.5V DC
- **Gain:** 26 dB
- **Built-in LP Filter:** ~5 kHz cutoff
- **Size:** Compact 3-pin module

**Pinout:**
```

┌─────────────┐
│ G   V   S   │  (Top view)
└─────────────┘
│   │   │
│   │   └─── Signal (PWM from GPIO 1)
│   └─────── Voltage (5V)
└─────────── Ground

```

**Advantages:**
- ✅ All-in-one solution (amp + filter)
- ✅ 3-pin simplicity
- ✅ Cheap (~$1-2)
- ✅ Sufficient power for small speakers

**Disadvantages:**
- ⚠️ 5V power required
- ⚠️ Fixed gain (no volume control)
- ⚠️ Not audiophile quality

---

### Audio Quality Comparison

| Method | Bit Depth | SNR | THD | Complexity | Cost |
|--------|-----------|-----|-----|------------|------|
| **I2S + PCM5102A** | 24-bit | 112 dB | 0.002% | Moderate | $5-10 |
| **PWM + RC Filter** | 9-bit | 80 dB | 0.8% | Simple | $0.50 |
| **PWM + 8002B** | 9-bit | 75 dB | 1.0% | Simple | $2 |

---

## 🖥️ Display Integration

### ST7789 TFT Display (1.47", 172x320)

**Wiring:**
```

ESP32-C6          ST7789 Display
─────────────────────────────────
GPIO 6 (MOSI) ──> DIN (SDA)
GPIO 7 (SCLK) ──> CLK (SCL)
GPIO 14       ──> CS
GPIO 15       ──> DC
GPIO 21       ──> RES (RST)
GPIO 22       ──> BL (Backlight)
3.3V          ──> VCC
GND           ──> GND

```

**SPI Configuration:**
- **Mode:** SPI Mode 0 (CPOL=0, CPHA=0)
- **Speed:** 40 MHz (default)
- **Data:** 8-bit, MSB first

**Backlight Control:**
```

// PWM backlight dimming
ledcAttach(PIN_BL, 1000, 8);  // 1 kHz, 8-bit
ledcWrite(PIN_BL, 180);        // 70% brightness

```

**Display Orientation:**
```

tft.setRotation(0);  // Portrait (172x320)
tft.setRotation(1);  // Landscape (320x172)
tft.setRotation(2);  // Portrait inverted
tft.setRotation(3);  // Landscape inverted

```

**Optional:** Display can be disabled in `AudioConfig.h`:
```

\#define DISPLAY_ENABLED 0  // Disable display

```

---

## ⚡ Power Requirements

### Power Consumption

**ESP32-C6 Only:**
- **Idle:** 20-30 mA @ 3.3V
- **Active (WiFi off):** 80-100 mA @ 3.3V
- **Active (WiFi on):** 150-200 mA @ 3.3V

**With Peripherals:**
| Component | Current Draw | Voltage |
|-----------|--------------|---------|
| ESP32-C6 | 80-100 mA | 3.3V |
| ST7789 Display | 20-30 mA | 3.3V |
| 8002B Amplifier | 50-200 mA | 5V |
| Speaker (8Ω, 0.5W) | 250 mA (peak) | 5V |
| **Total** | **~500 mA** | **5V** |

### Power Supply Options

**Option 1: USB Power (Recommended)**
```

USB 5V ──> ESP32-C6 Dev Board ──> Internal regulator ──> 3.3V
│
└──> 8002B Module (5V)

```
- ✅ Simple, no external components
- ✅ Sufficient for development
- ⚠️ USB port must provide 500 mA minimum

**Option 2: External 5V Supply**
```

5V Power Supply (1A)
├──> ESP32-C6 Vin
└──> 8002B Module V

```
- ✅ Reliable for production
- ✅ More current available
- Use quality adapter (1A minimum)

**Option 3: Battery (Portable)**
```

3x AA Batteries (4.5V) ──> Boost Converter ──> 5V ──> System
or
Li-Ion 3.7V ──> Boost Converter ──> 5V ──> System

```
- ⚠️ Requires boost converter module
- ⚠️ Battery life: 3-6 hours (typical)

### Power Optimization Tips

1. **Disable WiFi/Bluetooth:**
```

WiFi.mode(WIFI_OFF);
btStop();

```
Saves ~50-80 mA

2. **Reduce Display Brightness:**
```

ledcWrite(PIN_BL, 100);  // Lower brightness

```
Saves ~10-20 mA

3. **Use I2S Mode:**
```

PWM: 18% CPU → 100 mA
I2S: 7% CPU → 85 mA

```
Saves ~15 mA

4. **Deep Sleep (when idle):**
```

esp_deep_sleep_start();

```
Power: < 10 µA (not compatible with Audio OS currently)

---

## 🔧 Troubleshooting Hardware Issues

### No Audio Output

**Check 1: GPIO Connection**
```


# Test PWM output

audio test 440 1000  \# Should produce tone

```
- Use multimeter to check voltage on GPIO 1
- Should vary between 0V and 3.3V

**Check 2: Speaker/Amplifier**
- Connect GPIO 1 to 3.3V directly
- Should hear loud hum/buzz
- If not, amplifier or speaker faulty

**Check 3: Mode Configuration**
```

audio info

# Check "Audio Mode: I2S" or "PWM"

```

---

### Distorted/Noisy Audio

**Issue 1: Weak RC Filter**
- Increase capacitor: 100nF → 220nF
- Lower resistor: 1kΩ → 470Ω
- Trade-off: More filtering = less treble

**Issue 2: Ground Loop**
```

Fix: Connect all grounds to single point
ESP32-C6 GND ─┬─ 8002B GND
└─ Speaker GND (if separate power)

```

**Issue 3: Power Supply Noise**
- Use shielded USB cable
- Add 100µF capacitor across 5V and GND
- Use linear regulator instead of switching

---

### Display Not Working

**Check 1: SPI Wiring**
- Verify MOSI (GPIO 6) and SCLK (GPIO 7)
- Check CS (GPIO 14), DC (GPIO 15), RST (GPIO 21)

**Check 2: Power**
- ST7789 needs 3.3V (not 5V!)
- Check voltage with multimeter

**Check 3: Backlight**
- GPIO 22 should output PWM
- Test: `ledcWrite(PIN_BL, 255);`

**Check 4: Library Version**
```

Adafruit ST7789 must be installed
Tools → Manage Libraries → Adafruit ST7789

```

---

### ESP32-C6 Not Recognized (USB)

**Issue: Driver not installed**

**Windows:**
1. Download CP210x driver from Silicon Labs
2. Install and restart

**Linux:**
```

sudo usermod -a -G dialout \$USER

# Logout and login

```

**macOS:**
- Should work automatically with CH340 driver

**Check Port:**
```

Windows: Device Manager → Ports (COM4, COM5, etc.)
Linux: ls /dev/ttyUSB*
macOS: ls /dev/cu.usbserial*

```

---

### Overheating

**Normal Operating Temperature:**
- ESP32-C6: 40-60°C (warm to touch)
- 8002B: 50-70°C (hot but safe)

**If > 80°C:**
1. Check for short circuits
2. Reduce speaker volume
3. Add heatsink to 8002B
4. Improve ventilation

---

### Brown-Out Reset Loop

**Symptom:** ESP32-C6 keeps resetting

**Cause:** Insufficient power supply current

**Fix:**
1. Use USB port with 500+ mA capability
2. Use external 5V/1A power supply
3. Add 470µF capacitor across Vin and GND
4. Reduce display brightness
5. Disable WiFi

---

## 📐 PCB Design Guidelines (For Custom Boards)

### Recommended Layout

```

┌─────────────────────────────────────┐
│  ┌────────┐        ┌─────────┐     │
│  │ ESP32  │  USB   │ Display │     │
│  │  C6    │◄─────┐ │ ST7789  │     │
│  └────────┘      │ └─────────┘     │
│      │           │                  │
│  ┌───┴───┐   ┌───┴───┐             │
│  │8002B  │   │ USB-C │             │
│  │ Amp   │   │ Port  │             │
│  └───┬───┘   └───────┘             │
│      │                              │
│  ┌───┴───┐                          │
│  │Speaker│                          │
│  │ 8Ω    │                          │
│  └───────┘                          │
└─────────────────────────────────────┘

```

### Trace Width Guidelines

| Signal | Current | Width (1 oz copper) |
|--------|---------|---------------------|
| 3.3V Power | 200 mA | 0.3 mm (12 mil) |
| 5V Power | 500 mA | 0.5 mm (20 mil) |
| GND | Return | 1.0 mm (40 mil) |
| Audio (GPIO 1) | < 10 mA | 0.2 mm (8 mil) |
| SPI (MOSI/SCLK) | < 10 mA | 0.2 mm (8 mil) |

### Grounding

**Single-Point Ground:**
```

Digital GND (ESP32) ─┬─ Common GND Point
Analog GND (Audio)  ─┤
Display GND         ─┤
Power GND           ─┘

```

**Avoid:**
- Ground loops (multiple return paths)
- Long thin ground traces
- Mixing digital and analog grounds

---

## 📦 Bill of Materials (BOM)

### Minimal Setup (PWM Audio)

| Part | Quantity | Price (USD) |
|------|----------|-------------|
| ESP32-C6 Dev Board | 1 | $5-10 |
| 1kΩ Resistor | 1 | $0.10 |
| 100nF Capacitor | 1 | $0.10 |
| 8Ω Speaker (small) | 1 | $1-2 |
| USB Cable | 1 | $1 |
| **Total** | | **$7-13** |

### Full Setup (I2S Audio + Display)

| Part | Quantity | Price (USD) |
|------|----------|-------------|
| ESP32-C6 Dev Board | 1 | $5-10 |
| PCM5102A DAC Module | 1 | $5 |
| PAM8403 Amplifier | 1 | $1 |
| 8Ω Speaker | 1 | $2 |
| ST7789 1.47" Display | 1 | $8-12 |
| USB Cable | 1 | $1 |
| Jumper Wires | 10 | $1 |
| **Total** | | **$23-32** |

---

## 🔗 Recommended Suppliers

### Development Boards
- **AliExpress:** Cheapest, 2-4 week shipping
- **Adafruit:** Quality, fast shipping (US)
- **Mouser/DigiKey:** Industrial grade

### Modules
- **Adafruit:** I2S DAC breakouts, displays
- **SparkFun:** Audio modules
- **Amazon:** 8002B amplifiers (search "3W amplifier module")

### Components
- **LCSC:** Resistors, capacitors (bulk)
- **Mouser:** Quality components
- **Local electronics store:** Instant availability

---

## 📚 Datasheets

- **ESP32-C6:** [Espressif Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
- **PCM5102A DAC:** [TI Datasheet](https://www.ti.com/lit/ds/symlink/pcm5102a.pdf)
- **8002B Amplifier:** [Generic Chinese datasheet, search online]
- **ST7789 Display:** [Sitronix Datasheet](https://www.displayfuture.com/Display/datasheet/controller/ST7789.pdf)

---

## ✅ Final Checklist

**Before First Power-On:**
- [ ] All connections verified against wiring diagram
- [ ] No short circuits (multimeter continuity check)
- [ ] Power supply voltage correct (5V to Vin)
- [ ] GPIO pins not shorted to GND or 3.3V
- [ ] Speaker polarity correct (if polarized)

**After Power-On:**
- [ ] ESP32-C6 powers on (LED indicator if present)
- [ ] Serial console shows boot message (115200 baud)
- [ ] Display shows startup screen (if enabled)
- [ ] No smoke, no excessive heat
- [ ] Can type commands in serial console

**Audio Test:**
- [ ] `audio play` produces audible melody
- [ ] `audio stop` silences output
- [ ] `audio volume 255` increases loudness
- [ ] No distortion at moderate volume

**If All Checks Pass:**
🎉 **Hardware is working correctly!** 🎉

---

*Document Version: 1.0*  
*Last Updated: 2025-11-28*  
*For: ESP32 Audio OS v1.0*
```


***