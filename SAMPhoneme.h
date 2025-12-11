// SAMPhoneme.h - Phoneme Definitions and Text-to-Phoneme Conversion

#ifndef SAMPHONEME_H
#define SAMPHONEME_H

#include <Arduino.h>
#include "SAMCore.h"

// ============================================================================
// PHONEME DEFINITIONS (Original SAM phoneme set)
// ============================================================================

enum SAMPhonemeIndex {
    // Silence
    P_SILENCE = 0,
    
    // Vowels
    P_IY = 1,   // bEEt
    P_IH = 2,   // bIt
    P_EH = 3,   // bEt
    P_AE = 4,   // bAt
    P_AA = 5,   // bOt
    P_AH = 6,   // bUt
    P_AO = 7,   // bOUght
    P_UH = 8,   // bOOk
    P_UW = 9,   // bOOt
    P_ER = 10,  // bIRd
    
    // Diphthongs
    P_AY = 11,  // bIte
    P_AW = 12,  // bOUt
    P_OY = 13,  // bOY
    
    // Consonants - Stops
    P_P = 14,   // Pin
    P_B = 15,   // Bin
    P_T = 16,   // Tin
    P_D = 17,   // Din
    P_K = 18,   // Kin
    P_G = 19,   // Gin
    
    // Consonants - Fricatives
    P_F = 20,   // Fin
    P_V = 21,   // Vin
    P_TH = 22,  // THin
    P_DH = 23,  // THen
    P_S = 24,   // Sin
    P_Z = 25,   // Zen
    P_SH = 26,  // SHin
    P_ZH = 27,  // viSion
    P_H = 28,   // Hin
    
    // Consonants - Nasals
    P_M = 29,   // Min
    P_N = 30,   // Nin
    P_NG = 31,  // siNG
    
    // Consonants - Liquids
    P_L = 32,   // Lin
    P_R = 33,   // Run
    
    // Consonants - Glides
    P_W = 34,   // Win
    P_Y = 35,   // Yin
    
    // Affricates
    P_CH = 36,  // CHin
    P_J = 37,   // Jin
    
    P_MAX = 38
};

// ============================================================================
// PHONEME DATABASE
// ============================================================================

class SAMPhoneme {
public:
    // Text-to-phoneme conversion
    static size_t textToPhonemes(
        const char* text,
        struct SAMPhonemeData* phonemes,
        size_t maxPhonemes
    );
    
    // Get phoneme parameters (formants, duration, etc.)
    static void getPhonemeParams(
        uint8_t phonemeIndex,
        struct SAMPhonemeData& params
    );
    
    // Phoneme info
    static const char* getPhonemeName(uint8_t index);
    static uint8_t getPhonemeType(uint8_t index);
    
private:
    // Internal parsing functions
    static size_t parseWord(
        const char* word,
        struct SAMPhonemeData* phonemes,
        size_t maxPhonemes
    );
    
    static bool matchRule(
        const char* word,
        size_t pos,
        const char* pattern,
        const char* leftContext,
        const char* rightContext
    );
    
    static void addPhoneme(
        struct SAMPhonemeData* phonemes,
        size_t& count,
        size_t maxPhonemes,
        uint8_t phonemeIndex,
        uint8_t duration,
        uint8_t stress
    );
};

#endif // SAMPHONEME_H
