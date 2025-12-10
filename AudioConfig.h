// AudioConfig.h - ESP32 Audio OS v1.9
// Complete Configuration with LFO Modulation

#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

#include <Arduino.h>

// ============================================================================
// VERSION INFORMATION
// ============================================================================
#define AUDIO_OS_VERSION        "1.9.0"
#define AUDIO_OS_BUILD_DATE     __DATE__
#define SCHEMA_VERSION          "1.9"

// ============================================================================
// PERFORMANCE OPTIMIZATION FLAGS
// ============================================================================
#define USE_FIXED_POINT_MATH    1
#define USE_WAVETABLE_LOOKUP    1
#define WAVETABLE_SIZE          512

// ============================================================================
// MULTI-CORE SUPPORT FLAGS
// ============================================================================
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3)
  #define HAS_DUAL_CORE         1
  #define DEFAULT_AUDIO_CORE    0
  #define DEFAULT_UI_CORE       1
#else
  #define HAS_DUAL_CORE         0
  #define DEFAULT_AUDIO_CORE    0
  #define DEFAULT_UI_CORE       0
#endif

#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32S3)
  #define HAS_LP_CORE           1
#else
  #define HAS_LP_CORE           0
#endif

// ============================================================================
// AUDIO ENGINE DEFAULTS
// ============================================================================
#define DEFAULT_AUDIO_MODE      "i2s"
#define DEFAULT_SAMPLE_RATE     22050
#define DEFAULT_MAX_VOICES      4
#define DEFAULT_VOLUME          200

// ============================================================================
// I2S CONFIGURATION
// ============================================================================
#define DEFAULT_I2S_PIN         1
#define DEFAULT_I2S_BUFFER      128
#define DEFAULT_I2S_BUFFERS     4
#define DEFAULT_I2S_AMPLITUDE   12000

// ============================================================================
// PWM CONFIGURATION
// ============================================================================
#define DEFAULT_PWM_PIN         2
#define DEFAULT_PWM_FREQUENCY   78125
#define DEFAULT_PWM_RESOLUTION  9
#define DEFAULT_PWM_AMPLITUDE   5000
#define DEFAULT_PWM_GAIN        7

// ============================================================================
// ENVELOPE CONFIGURATION
// ============================================================================
#define ENV_ATTACK_SAMPLES      441
#define ENV_DECAY_SAMPLES       882
#define ENV_RELEASE_SAMPLES     1764
#define ENV_SUSTAIN_LEVEL       200

// ============================================================================
// EFFECTS DEFAULTS
// ============================================================================
#define DEFAULT_EQ_BASS         0
#define DEFAULT_EQ_MID          0
#define DEFAULT_EQ_TREBLE       0

// DELAY/ECHO DEFAULTS
#define DEFAULT_DELAY_ENABLED   false
#define DEFAULT_DELAY_TIME      250
#define DEFAULT_DELAY_FEEDBACK  50
#define DEFAULT_DELAY_MIX       30
#define MAX_DELAY_TIME          1000
#define DELAY_BUFFER_SIZE       44100

// BIQUAD EQ DEFAULTS
#define DEFAULT_EQ_ENABLED      false
#define EQ_BASS_FREQ            120
#define EQ_MID_FREQ             1000
#define EQ_TREBLE_FREQ          8000
#define EQ_Q_FACTOR             0.707f
#define EQ_MAX_GAIN_DB          12

// STATE-VARIABLE FILTER DEFAULTS
#define DEFAULT_FILTER_ENABLED  false
#define DEFAULT_FILTER_TYPE     0
#define DEFAULT_FILTER_CUTOFF   1000.0f
#define DEFAULT_FILTER_RESONANCE 0.1f
#define FILTER_CUTOFF_MIN       20.0f
#define FILTER_CUTOFF_MAX       20000.0f
#define FILTER_RESONANCE_MIN    0.0f
#define FILTER_RESONANCE_MAX    0.99f

// SCHROEDER REVERB DEFAULTS
#define DEFAULT_REVERB_ENABLED  false
#define DEFAULT_REVERB_ROOM_SIZE 0.5f
#define DEFAULT_REVERB_DAMPING  0.5f
#define DEFAULT_REVERB_WET      0.33f

// Schroeder Reverb: Comb filter delays (in samples @ 22.05kHz)
#define REVERB_COMB1_DELAY      1116
#define REVERB_COMB2_DELAY      1188
#define REVERB_COMB3_DELAY      1277
#define REVERB_COMB4_DELAY      1356

// Schroeder Reverb: Allpass filter delays (in samples @ 22.05kHz)
#define REVERB_ALLPASS1_DELAY   556
#define REVERB_ALLPASS2_DELAY   441

// Total reverb buffer size
#define REVERB_BUFFER_SIZE      (REVERB_COMB1_DELAY + REVERB_COMB2_DELAY + \
                                 REVERB_COMB3_DELAY + REVERB_COMB4_DELAY + \
                                 REVERB_ALLPASS1_DELAY + REVERB_ALLPASS2_DELAY)

// LFO MODULATION DEFAULTS (NEW!)
#define DEFAULT_LFO_ENABLED     false
#define DEFAULT_LFO_VIBRATO_ENABLED false
#define DEFAULT_LFO_TREMOLO_ENABLED false
#define DEFAULT_LFO_RATE        5.0f       // Hz (0.1-20 Hz)
#define DEFAULT_LFO_DEPTH       20.0f      // % (0-100%)
#define LFO_RATE_MIN            0.1f       // Hz
#define LFO_RATE_MAX            20.0f      // Hz
#define LFO_DEPTH_MIN           0.0f       // %
#define LFO_DEPTH_MAX           100.0f     // %

// ============================================================================
// RESAMPLER CONFIGURATION
// ============================================================================
#define DEFAULT_RESAMPLE_QUALITY "best"

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================
#define DISPLAY_ENABLED         1

#if DISPLAY_ENABLED
  #define PIN_SCLK              7
  #define PIN_MOSI              6
  #define PIN_TFT_CS            14
  #define PIN_TFT_DC            15
  #define PIN_TFT_RST           21
  #define PIN_BL                22
  #define LCD_WIDTH             172
  #define LCD_HEIGHT            320
  #define DEFAULT_BRIGHTNESS    180
  
  #define TFT_CS                PIN_TFT_CS
  #define TFT_DC                PIN_TFT_DC
  #define TFT_RST               PIN_TFT_RST
#endif

// ============================================================================
// FILESYSTEM CONFIGURATION
// ============================================================================
#define FS_MOUNT_POINT          "/spiffs"
#define FS_FORMAT_ON_FAIL       false

#define PROFILE_PATH            "/profiles"
#define CONFIG_PATH             "/config"
#define AUDIO_PATH              "/audio"
#define MELODY_PATH             "/melodies"

#define PATH_CONFIG             CONFIG_PATH
#define PATH_PROFILES           PROFILE_PATH
#define PATH_CODECS             "/codecs"
#define PATH_AUDIO              AUDIO_PATH
#define PATH_MELODIES           MELODY_PATH

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================
#define SERIAL_BAUD_RATE        115200
#define CONSOLE_BUFFER_SIZE     256
#define CONSOLE_MAX_CMD_LEN     256
#define CONSOLE_PROMPT          "audio> "
#define MAX_PROFILE_NAME        32
#define MAX_PROFILE_DESC        128

// ============================================================================
// VOLUME LIMITS
// ============================================================================
#define MIN_VOLUME              0
#define MAX_VOLUME              255

// ============================================================================
// PROFILE SCHEMA VERSION
// ============================================================================
#define PROFILE_SCHEMA_VERSION  SCHEMA_VERSION

// ============================================================================
// DEBUG & LOGGING
// ============================================================================
#define DEBUG_VERBOSE           0

// ============================================================================
// MEMORY LIMITS
// ============================================================================
#define MAX_CODEC_PLUGINS       8
#define MAX_AUDIO_FILES         64
#define JSON_DOC_SIZE           2048

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================
#define ENABLE_CPU_MONITOR      1
#define CPU_MONITOR_INTERVAL    1000

// ============================================================================
// ESP32 VARIANT STRING
// ============================================================================
#if defined(CONFIG_IDF_TARGET_ESP32)
  #define ESP32_VARIANT "ESP32"
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  #define ESP32_VARIANT "ESP32-S3"
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  #define ESP32_VARIANT "ESP32-C3"
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
  #define ESP32_VARIANT "ESP32-C6"
#else
  #define ESP32_VARIANT "Unknown"
#endif

#define ESP32_HAS_DUAL_CORE     HAS_DUAL_CORE

// ============================================================================
// AUDIO MODE ALIASES
// ============================================================================
#define AUDIO_MODE_I2S          MODE_I2S
#define AUDIO_MODE_PWM          MODE_PWM

#endif // AUDIO_CONFIG_H
