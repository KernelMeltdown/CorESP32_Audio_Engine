// SAMPhonemes.h - Modern Phoneme Definitions for ESP32
// Based on IPA (International Phonetic Alphabet)
// No C64 legacy - Scientifically accurate formant values

#ifndef SAM_PHONEMES_H
#define SAM_PHONEMES_H

#include <Arduino.h>
#include "SAMEngine.h"

// ============================================================================
// Phoneme Index Definitions (Modern, not C64-based)
// ============================================================================

namespace SAMPhonemeIndex {
    // Silence
    constexpr uint8_t SILENCE = 0;
    constexpr uint8_t PAUSE_SHORT = 1;
    constexpr uint8_t PAUSE_LONG = 2;
    
    // Vowels (based on acoustic measurements, not C64 values)
    constexpr uint8_t IY = 10;  // "ee" as in "see"
    constexpr uint8_t IH = 11;  // "i" as in "sit"
    constexpr uint8_t EH = 12;  // "e" as in "bed"
    constexpr uint8_t AE = 13;  // "a" as in "cat"
    constexpr uint8_t AA = 14;  // "o" as in "hot"
    constexpr uint8_t AO = 15;  // "aw" as in "law"
    constexpr uint8_t UH = 16;  // "u" as in "put"
    constexpr uint8_t UW = 17;  // "oo" as in "food"
    constexpr uint8_t ER = 18;  // "er" as in "bird"
    constexpr uint8_t AH = 19;  // "u" as in "but"
    constexpr uint8_t AX = 20;  // "a" as in "about" (schwa)
    
    // Diphthongs
    constexpr uint8_t EY = 25;  // "a" as in "day"
    constexpr uint8_t AY = 26;  // "i" as in "my"
    constexpr uint8_t OY = 27;  // "oy" as in "boy"
    constexpr uint8_t AW = 28;  // "ow" as in "how"
    constexpr uint8_t OW = 29;  // "o" as in "go"
    
    // Consonants - Stops
    constexpr uint8_t P = 40;   // "p" as in "put"
    constexpr uint8_t B = 41;   // "b" as in "but"
    constexpr uint8_t T = 42;   // "t" as in "top"
    constexpr uint8_t D = 43;   // "d" as in "dog"
    constexpr uint8_t K = 44;   // "k" as in "cat"
    constexpr uint8_t G = 45;   // "g" as in "got"
    
    // Consonants - Fricatives
    constexpr uint8_t F = 50;   // "f" as in "fan"
    constexpr uint8_t V = 51;   // "v" as in "van"
    constexpr uint8_t TH = 52;  // "th" as in "thin"
    constexpr uint8_t DH = 53;  // "th" as in "then"
    constexpr uint8_t S = 54;   // "s" as in "sit"
    constexpr uint8_t Z = 55;   // "z" as in "zoo"
    constexpr uint8_t SH = 56;  // "sh" as in "shop"
    constexpr uint8_t ZH = 57;  // "zh" as in "measure"
    constexpr uint8_t H = 58;   // "h" as in "hot"
    
    // Consonants - Affricates
    constexpr uint8_t CH = 60;  // "ch" as in "church"
    constexpr uint8_t JH = 61;  // "j" as in "judge"
    
    // Consonants - Nasals
    constexpr uint8_t M = 70;   // "m" as in "man"
    constexpr uint8_t N = 71;   // "n" as in "not"
    constexpr uint8_t NG = 72;  // "ng" as in "sing"
    
    // Consonants - Liquids
    constexpr uint8_t L = 80;   // "l" as in "let"
    constexpr uint8_t R = 81;   // "r" as in "red"
    
    // Consonants - Glides
    constexpr uint8_t W = 90;   // "w" as in "wet"
    constexpr uint8_t Y = 91;   // "y" as in "yes"
}

// ============================================================================
// Formant Data Tables (ESP32-optimized, scientifically accurate)
// ============================================================================

struct PhonemeFormantData {
    const char* symbol;
    PhonemeType type;
    FormantSet formants;
    uint16_t baseDuration_ms;
    float noiseLevel;        // 0.0-1.0 for fricatives
    float voicing;           // 0.0-1.0 (0=unvoiced, 1=fully voiced)
};

// Modern formant values based on acoustic research (not C64 approximations)
namespace SAMFormantTables {
    
    // Master Phoneme Lookup Table - declared in .cpp file
    extern const PhonemeFormantData PHONEME_TABLE[128];
    
} // namespace SAMFormantTables

// ============================================================================
// Text-to-Phoneme Rules (Modern English, not C64-based)
// ============================================================================

namespace SAMTextRules {
    
    struct PhonemeRule {
        const char* pattern;     // Text pattern to match
        const char* context;     // Context (left/right conditions)
        const char* phonemes;    // Output phoneme string
        uint16_t priority;       // Higher = check first
    };
    
    // Initialize rule system
    void initializeRules();
    
    // Convert text to phonemes using rules
    String textToPhonemes(const String& text);
    
    // Dictionary lookup for special words
    bool lookupDictionary(const String& word, String& phonemes);
    
    // Add custom dictionary entry
    void addDictionaryEntry(const String& word, const String& phonemes);
}

#endif // SAM_PHONEMES_H