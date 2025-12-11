/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM PHONEME DEFINITIONS - Header                                            ║
 ║  Modern phoneme data based on IPA and acoustic research                    ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#ifndef SAM_PHONEMES_H
#define SAM_PHONEMES_H

#include <Arduino.h>
#include "SAMEngine.h"

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Type Classification
// ═══════════════════════════════════════════════════════════════════════════

enum class PhonemeType : uint8_t {
    SILENCE = 0,
    VOWEL,
    DIPHTHONG,
    STOP,
    FRICATIVE,
    AFFRICATE,
    NASAL,
    LIQUID,
    GLIDE
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Data Structure
// ═══════════════════════════════════════════════════════════════════════════

struct PhonemeFormantData {
    const char* symbol;
    const char* example;
    PhonemeType type;
    FormantSet formants;
    uint8_t defaultDuration;
    bool voiced;
};

// ═══════════════════════════════════════════════════════════════════════════
// Formant Tables (Modern, Research-Based)
// ═══════════════════════════════════════════════════════════════════════════

namespace SAMFormantTables {
    // Helper function to create FormantSet
    inline FormantSet makeFormantSet(float f1, float f2, float f3,
                                     float a1, float a2, float a3,
                                     float b1, float b2, float b3) {
        FormantSet fs;
        fs.f1_freq = f1; fs.f2_freq = f2; fs.f3_freq = f3;
        fs.f1_amp = a1;  fs.f2_amp = a2;  fs.f3_amp = a3;
        fs.f1_bw = b1;   fs.f2_bw = b2;   fs.f3_bw = b3;
        return fs;
    }

    // VOWELS (based on Peterson & Barney 1952)
    extern const PhonemeFormantData SILENCE_DATA;
    extern const PhonemeFormantData IY_DATA;  // "ee" in "see"
    extern const PhonemeFormantData IH_DATA;  // "i" in "sit"
    extern const PhonemeFormantData EH_DATA;  // "e" in "bed"
    extern const PhonemeFormantData AE_DATA;  // "a" in "cat"
    extern const PhonemeFormantData AH_DATA;  // "u" in "but"
    extern const PhonemeFormantData AX_DATA;  // schwa "a" in "about"
    extern const PhonemeFormantData ER_DATA;  // "er" in "bird"
    extern const PhonemeFormantData AA_DATA;  // "o" in "hot"
    extern const PhonemeFormantData AO_DATA;  // "aw" in "law"
    extern const PhonemeFormantData UH_DATA;  // "u" in "put"
    extern const PhonemeFormantData UW_DATA;  // "oo" in "food"
    
    // DIPHTHONGS
    extern const PhonemeFormantData EY_DATA;  // "a" in "day"
    extern const PhonemeFormantData AY_DATA;  // "i" in "my"
    extern const PhonemeFormantData OY_DATA;  // "oy" in "boy"
    extern const PhonemeFormantData AW_DATA;  // "ow" in "how"
    extern const PhonemeFormantData OW_DATA;  // "o" in "go"
    
    // STOPS (voiceless)
    extern const PhonemeFormantData P_DATA;   // "p" in "put"
    extern const PhonemeFormantData B_DATA;   // "b" in "but"
    extern const PhonemeFormantData T_DATA;   // "t" in "top"
    extern const PhonemeFormantData D_DATA;   // "d" in "dog"
    extern const PhonemeFormantData K_DATA;   // "k" in "cat"
    extern const PhonemeFormantData G_DATA;   // "g" in "got"
    
    // FRICATIVES
    extern const PhonemeFormantData F_DATA;   // "f" in "fan"
    extern const PhonemeFormantData V_DATA;   // "v" in "van"
    extern const PhonemeFormantData TH_DATA;  // "th" in "thin"
    extern const PhonemeFormantData DH_DATA;  // "th" in "then"
    extern const PhonemeFormantData S_DATA;   // "s" in "sit"
    extern const PhonemeFormantData Z_DATA;   // "z" in "zoo"
    extern const PhonemeFormantData SH_DATA;  // "sh" in "shop"
    extern const PhonemeFormantData ZH_DATA;  // "zh" in "measure"
    extern const PhonemeFormantData H_DATA;   // "h" in "hot"
    
    // AFFRICATES
    extern const PhonemeFormantData CH_DATA;  // "ch" in "church"
    extern const PhonemeFormantData JH_DATA;  // "j" in "judge"
    
    // NASALS
    extern const PhonemeFormantData M_DATA;   // "m" in "man"
    extern const PhonemeFormantData N_DATA;   // "n" in "not"
    extern const PhonemeFormantData NG_DATA;  // "ng" in "sing"
    
    // LIQUIDS
    extern const PhonemeFormantData L_DATA;   // "l" in "let"
    extern const PhonemeFormantData R_DATA;   // "r" in "red"
    
    // GLIDES
    extern const PhonemeFormantData W_DATA;   // "w" in "wet"
    extern const PhonemeFormantData Y_DATA;   // "y" in "yes"
}

// ═══════════════════════════════════════════════════════════════════════════
// Text-to-Phoneme Rules
// ═══════════════════════════════════════════════════════════════════════════

namespace SAMTextRules {
    // Initialize phoneme dictionary
    void initializeRules();
    
    // Convert text to phonemes
    String textToPhonemes(const String& text);
    
    // Dictionary lookup
    bool lookupDictionary(const String& word, String& phonemes);
    
    // Add custom dictionary entry
    void addDictionaryEntry(const String& word, const String& phonemes);
}

#endif // SAM_PHONEMES_H