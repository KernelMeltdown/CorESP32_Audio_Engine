// SAMConfig.h - SAM Speech Synthesis Configuration for ESP32
// Bare-Metal optimized, C64-free implementation

#ifndef SAMCONFIG_H
#define SAMCONFIG_H

// ============================================================================
// SAM CORE PARAMETERS
// ============================================================================

// Voice parameters (0-255 range for compatibility)
#define SAM_DEFAULT_SPEED      72    // 40-150 (72 = natural)
#define SAM_DEFAULT_PITCH      64    // 20-120 (64 = neutral)
#define SAM_DEFAULT_THROAT     128   // 90-180 (128 = neutral)
#define SAM_DEFAULT_MOUTH      128   // 90-180 (128 = neutral)

// Parameter limits
#define SAM_SPEED_MIN          40
#define SAM_SPEED_MAX          150
#define SAM_PITCH_MIN          20
#define SAM_PITCH_MAX          120
#define SAM_THROAT_MIN         90
#define SAM_THROAT_MAX         180
#define SAM_MOUTH_MIN          90
#define SAM_MOUTH_MAX          180

// ============================================================================
// AUDIO CONFIGURATION
// ============================================================================

// Sample rate (44.1kHz for high quality, NOT C64's 22.05kHz!)
#define SAM_SAMPLE_RATE        44100

// Output format
#define SAM_BIT_DEPTH          16
#define SAM_CHANNELS           1     // Mono

// Buffer sizes
#define SAM_RENDER_BUFFER      128   // Match AudioEngine's I2S buffer
#define SAM_PHONEME_BUFFER     256   // Max phonemes per synthesis

// ============================================================================
// OPTIMIZATION FLAGS
// ============================================================================

// Use ESP32 hardware FPU (NO lookup tables!)
#define SAM_USE_FPU            1

// Use fixed-point math where beneficial
#define SAM_USE_FIXED_POINT    USE_FIXED_POINT_MATH  // From AudioConfig.h

// Enable SIMD optimizations (ESP32-S3)
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  #define SAM_USE_SIMD         1
#else
  #define SAM_USE_SIMD         0
#endif

// ============================================================================
// DSP ENHANCEMENTS (from your HTML demo)
// ============================================================================

// Enable smoothing filter
#define SAM_ENABLE_SMOOTHING   1
#define SAM_SMOOTH_AMOUNT      35    // 0-100 (35 = balanced)

// Enable cubic interpolation
#define SAM_ENABLE_INTERPOLATION 1
#define SAM_INTERP_AMOUNT      40    // 0-100 (40 = smooth)

// Enable formant boosting
#define SAM_ENABLE_FORMANT_BOOST 1
#define SAM_FORMANT_BOOST      15    // 0-50 (15 = clarity)

// Enable bass control
#define SAM_ENABLE_BASS_CONTROL 1
#define SAM_BASS_DB            0     // -10 to +10 dB

// ============================================================================
// PHONEME DEFINITIONS
// ============================================================================

// Total phonemes (original SAM set)
#define SAM_PHONEME_COUNT      64

// Phoneme types
#define PHONEME_SILENCE        0
#define PHONEME_VOWEL          1
#define PHONEME_CONSONANT      2
#define PHONEME_NASAL          3

// ============================================================================
// FORMANT SYNTHESIS
// ============================================================================

// Number of formants
#define SAM_FORMANT_COUNT      3

// Formant frequencies (Hz) - typical human speech
#define FORMANT_F1_MIN         200.0f
#define FORMANT_F1_MAX         1000.0f
#define FORMANT_F2_MIN         800.0f
#define FORMANT_F2_MAX         3000.0f
#define FORMANT_F3_MIN         2000.0f
#define FORMANT_F3_MAX         4000.0f

// Formant amplitudes (0.0 - 1.0)
#define FORMANT_A1_DEFAULT     0.8f
#define FORMANT_A2_DEFAULT     0.6f
#define FORMANT_A3_DEFAULT     0.3f

// Bandwidth (Hz)
#define FORMANT_BW_DEFAULT     100.0f

// ============================================================================
// TIMING & TRANSITIONS
// ============================================================================

// Phoneme timing (samples per unit)
#define SAM_TIMING_BASE        (SAM_SAMPLE_RATE / 50)  // 20ms base unit

// Transition smoothing (samples)
#define SAM_TRANSITION_SAMPLES 64

// Stress markers
#define SAM_STRESS_PRIMARY     1
#define SAM_STRESS_SECONDARY   2
#define SAM_STRESS_NONE        0

// ============================================================================
// MEMORY LIMITS
// ============================================================================

// Maximum text length
#define SAM_MAX_TEXT_LENGTH    256

// Maximum output samples
#define SAM_MAX_OUTPUT_SAMPLES (SAM_SAMPLE_RATE * 10)  // 10 seconds max

// Stack usage estimate (bytes)
#define SAM_STACK_USAGE        4096

// Heap usage estimate (bytes)
#define SAM_HEAP_USAGE         8192

// ============================================================================
// DEBUG & PROFILING
// ============================================================================

// Enable debug output
#define SAM_DEBUG              0

// Enable performance profiling
#define SAM_PROFILE            0

#if SAM_PROFILE
  #define SAM_PROFILE_START()  uint32_t _sam_t0 = micros()
  #define SAM_PROFILE_END(msg) Serial.printf("[SAM] %s: %lu us\n", msg, micros() - _sam_t0)
#else
  #define SAM_PROFILE_START()
  #define SAM_PROFILE_END(msg)
#endif

// ============================================================================
// FIXED-POINT MATH (if enabled)
// ============================================================================

#if SAM_USE_FIXED_POINT
  typedef int32_t sam_fixed_t;
  #define SAM_FIXED_SHIFT      16
  #define SAM_FLOAT_TO_FIXED(f) ((sam_fixed_t)((f) * 65536.0f))
  #define SAM_FIXED_TO_FLOAT(x) ((float)(x) / 65536.0f)
  #define SAM_FIXED_MUL(a, b)   (((int64_t)(a) * (int64_t)(b)) >> SAM_FIXED_SHIFT)
#endif

#endif // SAMCONFIG_H
