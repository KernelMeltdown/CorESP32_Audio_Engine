// AudioConfig.h - ESP32 Audio OS v2.0
// Hardware Configuration and Limits

#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

#include <Arduino.h>

// ============================================================================
// VERSION INFORMATION
// ============================================================================
#define AUDIO_OS_VERSION        "2.0.0"
#define AUDIO_OS_BUILD_DATE     __DATE__
#define SCHEMA_VERSION          "2.0"

// ============================================================================
// HARDWARE DETECTION
// ============================================================================
#if defined(CONFIG_IDF_TARGET_ESP32)
  #define ESP32_VARIANT         "ESP32"
  #define HAS_DUAL_CORE         1
  #define HAS_LP_CORE           0
  #define DEFAULT_AUDIO_CORE    0
  #define DEFAULT_UI_CORE       1
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  #define ESP32_VARIANT         "ESP32-S3"
  #define HAS_DUAL_CORE         1
  #define HAS_LP_CORE           0
  #define DEFAULT_AUDIO_CORE    0
  #define DEFAULT_UI_CORE       1
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  #define ESP32_VARIANT         "ESP32-C3"
  #define HAS_DUAL_CORE         0
  #define HAS_LP_CORE           0
  #define DEFAULT_AUDIO_CORE    0
  #define DEFAULT_UI_CORE       0
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
  #define ESP32_VARIANT         "ESP32-C6"
  #define HAS_DUAL_CORE         0
  #define HAS_LP_CORE           1
  #define DEFAULT_AUDIO_CORE    0
  #define DEFAULT_UI_CORE       0
#else
  #define ESP32_VARIANT         "Unknown"
  #define HAS_DUAL_CORE         0
  #define HAS_LP_CORE           0
  #define DEFAULT_AUDIO_CORE    0
  #define DEFAULT_UI_CORE       0
#endif

// ============================================================================
// PERFORMANCE FLAGS
// ============================================================================
#define USE_FIXED_POINT_MATH    1
#define USE_WAVETABLE_LOOKUP    1
#define WAVETABLE_SIZE          512

// ============================================================================
// DEFAULT VALUES (Used by AudioSettings.h constructors)
// ============================================================================
#define DEFAULT_SAMPLE_RATE         22050
#define DEFAULT_MAX_VOICES          4
#define DEFAULT_VOLUME              200

#define DEFAULT_I2S_PIN             1
#define DEFAULT_I2S_BUFFER          128
#define DEFAULT_I2S_BUFFERS         4
#define DEFAULT_I2S_AMPLITUDE       12000

#define DEFAULT_PWM_PIN             2
#define DEFAULT_PWM_FREQUENCY       78125
#define DEFAULT_PWM_RESOLUTION      9
#define DEFAULT_PWM_AMPLITUDE       5000
#define DEFAULT_PWM_GAIN            7

#define DEFAULT_EQ_ENABLED          false
#define DEFAULT_EQ_BASS             0
#define DEFAULT_EQ_MID              0
#define DEFAULT_EQ_TREBLE           0

#define DEFAULT_FILTER_ENABLED      false
#define DEFAULT_FILTER_TYPE         0
#define DEFAULT_FILTER_CUTOFF       1000.0f
#define DEFAULT_FILTER_RESONANCE    0.1f

#define DEFAULT_REVERB_ENABLED      false
#define DEFAULT_REVERB_ROOM_SIZE    0.5f
#define DEFAULT_REVERB_DAMPING      0.5f
#define DEFAULT_REVERB_WET          0.33f

#define DEFAULT_LFO_ENABLED         false
#define DEFAULT_LFO_VIBRATO_ENABLED false
#define DEFAULT_LFO_TREMOLO_ENABLED false
#define DEFAULT_LFO_RATE            5.0f
#define DEFAULT_LFO_DEPTH           20.0f

#define DEFAULT_DELAY_ENABLED       false
#define DEFAULT_DELAY_TIME          250
#define DEFAULT_DELAY_FEEDBACK      50
#define DEFAULT_DELAY_MIX           30

// ============================================================================
// SYSTEM LIMITS
// ============================================================================
#define MAX_CODEC_PLUGINS       16
#define MAX_AUDIO_FILES         128
#define MAX_VOICES              8
#define JSON_DOC_SIZE           4096
#define SERIAL_BAUD_RATE        115200
#define CONSOLE_BUFFER_SIZE     256
#define CONSOLE_MAX_CMD_LEN     256
#define CONSOLE_PROMPT          "audio> "
#define MAX_PROFILE_NAME        32
#define MAX_PROFILE_DESC        128

// ============================================================================
// FILESYSTEM PATHS (LittleFS)
// ============================================================================
#define FS_MOUNT_POINT          "/littlefs"
#define FS_FORMAT_ON_FAIL       false

#define PATH_CONFIG             "/config"
#define PATH_PROFILES           "/profiles"
#define PATH_CODECS             "/codecs"
#define PATH_AUDIO              "/audio"
#define PATH_MELODIES           "/melodies"
#define PATH_SYSTEM_CONFIG      "/config/system.json"

// ============================================================================
// AUDIO LIMITS
// ============================================================================
#define MIN_VOLUME              0
#define MAX_VOLUME              255
#define MIN_SAMPLE_RATE         8000
#define MAX_SAMPLE_RATE         96000

// ============================================================================
// ENVELOPE PARAMETERS
// ============================================================================
#define ENV_ATTACK_SAMPLES      441
#define ENV_DECAY_SAMPLES       882
#define ENV_RELEASE_SAMPLES     1764
#define ENV_SUSTAIN_LEVEL       200

// ============================================================================
// EFFECT BUFFER SIZES
// ============================================================================
#define MAX_DELAY_TIME          1000
#define DELAY_BUFFER_SIZE       44100

#define REVERB_COMB1_DELAY      1116
#define REVERB_COMB2_DELAY      1188
#define REVERB_COMB3_DELAY      1277
#define REVERB_COMB4_DELAY      1356
#define REVERB_ALLPASS1_DELAY   556
#define REVERB_ALLPASS2_DELAY   441
#define REVERB_BUFFER_SIZE      (REVERB_COMB1_DELAY + REVERB_COMB2_DELAY + \
                                 REVERB_COMB3_DELAY + REVERB_COMB4_DELAY + \
                                 REVERB_ALLPASS1_DELAY + REVERB_ALLPASS2_DELAY)

// ============================================================================
// EQ PARAMETERS
// ============================================================================
#define EQ_BASS_FREQ            120
#define EQ_MID_FREQ             1000
#define EQ_TREBLE_FREQ          8000
#define EQ_Q_FACTOR             0.707f
#define EQ_MAX_GAIN_DB          12

// ============================================================================
// FILTER PARAMETERS
// ============================================================================
#define FILTER_CUTOFF_MIN       20.0f
#define FILTER_CUTOFF_MAX       20000.0f
#define FILTER_RESONANCE_MIN    0.0f
#define FILTER_RESONANCE_MAX    0.99f

// ============================================================================
// LFO PARAMETERS
// ============================================================================
#define LFO_RATE_MIN            0.1f
#define LFO_RATE_MAX            20.0f
#define LFO_DEPTH_MIN           0.0f
#define LFO_DEPTH_MAX           100.0f

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================
#define ENABLE_CPU_MONITOR      1
#define CPU_MONITOR_INTERVAL    1000

// ============================================================================
// COMPATIBILITY ALIASES
// ============================================================================
#define AUDIO_MODE_I2S          MODE_I2S
#define AUDIO_MODE_PWM          MODE_PWM
#define PROFILE_SCHEMA_VERSION  SCHEMA_VERSION
#define ESP32_HAS_DUAL_CORE     HAS_DUAL_CORE

// ============================================================================
// DEBUG & LOGGING
// ============================================================================
#define DEBUG_VERBOSE           0

#endif // AUDIO_CONFIG_H