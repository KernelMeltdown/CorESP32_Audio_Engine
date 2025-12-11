// SAMPhoneme.cpp - Phoneme Database and Text-to-Phoneme Conversion

#include "SAMPhoneme.h"
#include <ctype.h>
#include <string.h>

// ============================================================================
// PHONEME PARAMETER DATABASE
// ============================================================================

struct PhonemeData {
    const char* name;
    uint8_t type;
    uint8_t baseDuration;  // Base duration in timing units
    // Formant frequencies (Hz)
    float f1, f2, f3;
    // Formant amplitudes (0.0-1.0)
    float a1, a2, a3;
    // Bandwidth
    float bw;
};

static const PhonemeData phonemeDatabase[P_MAX] = {
    // name,     type,              dur, f1,   f2,   f3,   a1,  a2,  a3,  bw
    {"_",        PHONEME_SILENCE,    5,  0,    0,    0,    0,   0,   0,   0},     // Silence
    
    // Vowels
    {"IY",       PHONEME_VOWEL,     12, 270,  2290, 3010, 0.8, 0.6, 0.3, 100},  // bEEt
    {"IH",       PHONEME_VOWEL,     10, 390,  1990, 2550, 0.8, 0.6, 0.3, 100},  // bIt
    {"EH",       PHONEME_VOWEL,     11, 530,  1840, 2480, 0.8, 0.6, 0.3, 100},  // bEt
    {"AE",       PHONEME_VOWEL,     12, 660,  1720, 2410, 0.8, 0.6, 0.3, 100},  // bAt
    {"AA",       PHONEME_VOWEL,     13, 730,  1090, 2440, 0.8, 0.6, 0.3, 100},  // bOt
    {"AH",       PHONEME_VOWEL,     10, 640,  1190, 2390, 0.8, 0.6, 0.3, 100},  // bUt
    {"AO",       PHONEME_VOWEL,     13, 570,  840,  2410, 0.8, 0.6, 0.3, 100},  // bOUght
    {"UH",       PHONEME_VOWEL,     10, 440,  1020, 2240, 0.8, 0.6, 0.3, 100},  // bOOk
    {"UW",       PHONEME_VOWEL,     12, 300,  870,  2240, 0.8, 0.6, 0.3, 100},  // bOOt
    {"ER",       PHONEME_VOWEL,     12, 490,  1350, 1690, 0.8, 0.6, 0.3, 100},  // bIRd
    
    // Diphthongs
    {"AY",       PHONEME_VOWEL,     14, 660,  1720, 2410, 0.8, 0.6, 0.3, 100},  // bIte
    {"AW",       PHONEME_VOWEL,     14, 730,  1090, 2440, 0.8, 0.6, 0.3, 100},  // bOUt
    {"OY",       PHONEME_VOWEL,     14, 570,  840,  2410, 0.8, 0.6, 0.3, 100},  // bOY
    
    // Stops
    {"P",        PHONEME_CONSONANT,  8, 0,    0,    0,    0,   0,   0,   0},     // Pin
    {"B",        PHONEME_CONSONANT,  7, 0,    0,    0,    0,   0,   0,   0},     // Bin
    {"T",        PHONEME_CONSONANT,  8, 0,    0,    0,    0,   0,   0,   0},     // Tin
    {"D",        PHONEME_CONSONANT,  7, 0,    0,    0,    0,   0,   0,   0},     // Din
    {"K",        PHONEME_CONSONANT,  8, 0,    0,    0,    0,   0,   0,   0},     // Kin
    {"G",        PHONEME_CONSONANT,  7, 0,    0,    0,    0,   0,   0,   0},     // Gin
    
    // Fricatives
    {"F",        PHONEME_CONSONANT,  9, 0,    1400, 2500, 0,   0.4, 0.3, 200},  // Fin
    {"V",        PHONEME_CONSONANT,  8, 0,    1400, 2500, 0,   0.4, 0.3, 200},  // Vin
    {"TH",       PHONEME_CONSONANT,  9, 0,    1400, 2500, 0,   0.3, 0.2, 200},  // THin
    {"DH",       PHONEME_CONSONANT,  7, 0,    1400, 2500, 0,   0.3, 0.2, 200},  // THen
    {"S",        PHONEME_CONSONANT, 10, 0,    1800, 2500, 0,   0.5, 0.4, 300},  // Sin
    {"Z",        PHONEME_CONSONANT,  8, 0,    1800, 2500, 0,   0.5, 0.4, 300},  // Zen
    {"SH",       PHONEME_CONSONANT, 10, 0,    1800, 2400, 0,   0.5, 0.4, 300},  // SHin
    {"ZH",       PHONEME_CONSONANT,  8, 0,    1800, 2400, 0,   0.5, 0.4, 300},  // viSion
    {"H",        PHONEME_CONSONANT,  7, 0,    800,  2500, 0,   0.2, 0.1, 300},  // Hin
    
    // Nasals
    {"M",        PHONEME_NASAL,     10, 280,  1300, 2500, 0.7, 0.3, 0.2, 150},  // Min
    {"N",        PHONEME_NASAL,     10, 280,  1700, 2600, 0.7, 0.3, 0.2, 150},  // Nin
    {"NG",       PHONEME_NASAL,     11, 280,  2300, 2750, 0.7, 0.3, 0.2, 150},  // siNG
    
    // Liquids
    {"L",        PHONEME_CONSONANT, 10, 300,  1000, 2500, 0.7, 0.4, 0.2, 150},  // Lin
    {"R",        PHONEME_CONSONANT, 10, 490,  1350, 1690, 0.7, 0.5, 0.3, 150},  // Run
    
    // Glides
    {"W",        PHONEME_CONSONANT,  9, 300,  870,  2240, 0.6, 0.4, 0.2, 150},  // Win
    {"Y",        PHONEME_CONSONANT,  8, 270,  2290, 3010, 0.6, 0.4, 0.2, 150},  // Yin
    
    // Affricates
    {"CH",       PHONEME_CONSONANT, 11, 0,    1800, 2400, 0,   0.5, 0.4, 300},  // CHin
    {"J",        PHONEME_CONSONANT, 10, 0,    1800, 2400, 0,   0.5, 0.4, 300},  // Jin
};

// ============================================================================
// SIMPLE DICTIONARY (common words for better accuracy)
// ============================================================================

struct DictionaryEntry {
    const char* word;
    const uint8_t* phonemes;  // Terminated with P_SILENCE
};

static const uint8_t phon_hello[] = {P_H, P_EH, P_L, P_OY, P_SILENCE};
static const uint8_t phon_world[] = {P_W, P_ER, P_L, P_D, P_SILENCE};
static const uint8_t phon_esp32[] = {P_EH, P_S, P_P, P_TH, P_ER, P_T, P_IY, P_T, P_UW, P_SILENCE};
static const uint8_t phon_audio[] = {P_AO, P_D, P_IY, P_OY, P_SILENCE};

static const DictionaryEntry dictionary[] = {
    {"hello", phon_hello},
    {"world", phon_world},
    {"esp32", phon_esp32},
    {"audio", phon_audio},
    {nullptr, nullptr}
};

// ============================================================================
// PHONEME INTERFACE FUNCTIONS
// ============================================================================

void SAMPhoneme::getPhonemeParams(uint8_t phonemeIndex, struct SAMPhonemeData& params) {
    if (phonemeIndex >= P_MAX) {
        phonemeIndex = P_SILENCE;
    }
    
    const PhonemeData& data = phonemeDatabase[phonemeIndex];
    
    params.index = phonemeIndex;
    params.type = data.type;
    params.duration = data.baseDuration;
    params.stress = SAM_STRESS_NONE;
    
    params.f1 = data.f1;
    params.f2 = data.f2;
    params.f3 = data.f3;
    params.a1 = data.a1;
    params.a2 = data.a2;
    params.a3 = data.a3;
    params.bw = data.bw;
}

const char* SAMPhoneme::getPhonemeName(uint8_t index) {
    if (index >= P_MAX) return "?";
    return phonemeDatabase[index].name;
}

uint8_t SAMPhoneme::getPhonemeType(uint8_t index) {
    if (index >= P_MAX) return PHONEME_SILENCE;
    return phonemeDatabase[index].type;
}

// ============================================================================
// TEXT-TO-PHONEME CONVERSION
// ============================================================================

void SAMPhoneme::addPhoneme(
    struct SAMPhonemeData* phonemes,
    size_t& count,
    size_t maxPhonemes,
    uint8_t phonemeIndex,
    uint8_t duration,
    uint8_t stress
) {
    if (count >= maxPhonemes) return;
    
    getPhonemeParams(phonemeIndex, phonemes[count]);
    
    if (duration > 0) {
        phonemes[count].duration = duration;
    }
    phonemes[count].stress = stress;
    
    count++;
}

size_t SAMPhoneme::textToPhonemes(
    const char* text,
    struct SAMPhonemeData* phonemes,
    size_t maxPhonemes
) {
    if (!text || !phonemes || maxPhonemes == 0) {
        return 0;
    }
    
    size_t count = 0;
    char word[32];
    size_t wordLen = 0;
    
    // Convert to lowercase and parse words
    for (size_t i = 0; text[i] != '\0' && count < maxPhonemes; i++) {
        char c = tolower(text[i]);
        
        if (isalpha(c)) {
            if (wordLen < sizeof(word) - 1) {
                word[wordLen++] = c;
            }
        } else {
            // End of word
            if (wordLen > 0) {
                word[wordLen] = '\0';
                count += parseWord(word, phonemes + count, maxPhonemes - count);
                wordLen = 0;
                
                // Add pause after word
                if (count < maxPhonemes) {
                    addPhoneme(phonemes, count, maxPhonemes, P_SILENCE, 2, SAM_STRESS_NONE);
                }
            }
            
            // Handle punctuation
            if (c == '.' || c == '!' || c == '?') {
                // Longer pause
                if (count < maxPhonemes) {
                    addPhoneme(phonemes, count, maxPhonemes, P_SILENCE, 8, SAM_STRESS_NONE);
                }
            } else if (c == ',') {
                // Medium pause
                if (count < maxPhonemes) {
                    addPhoneme(phonemes, count, maxPhonemes, P_SILENCE, 5, SAM_STRESS_NONE);
                }
            }
        }
    }
    
    // Process last word
    if (wordLen > 0 && count < maxPhonemes) {
        word[wordLen] = '\0';
        count += parseWord(word, phonemes + count, maxPhonemes - count);
    }
    
    return count;
}

size_t SAMPhoneme::parseWord(
    const char* word,
    struct SAMPhonemeData* phonemes,
    size_t maxPhonemes
) {
    size_t count = 0;
    
    // Check dictionary first
    for (size_t i = 0; dictionary[i].word != nullptr; i++) {
        if (strcmp(word, dictionary[i].word) == 0) {
            // Found in dictionary
            for (size_t j = 0; dictionary[i].phonemes[j] != P_SILENCE && count < maxPhonemes; j++) {
                addPhoneme(phonemes, count, maxPhonemes, 
                          dictionary[i].phonemes[j], 0, SAM_STRESS_NONE);
            }
            return count;
        }
    }
    
    // Simple rule-based parsing (letter-to-sound)
    size_t len = strlen(word);
    
    for (size_t i = 0; i < len && count < maxPhonemes; i++) {
        char c = word[i];
        char next = (i + 1 < len) ? word[i + 1] : '\0';
        
        switch (c) {
            case 'a':
                if (next == 'y') {
                    addPhoneme(phonemes, count, maxPhonemes, P_AY, 0, SAM_STRESS_NONE);
                    i++;
                } else if (next == 'w') {
                    addPhoneme(phonemes, count, maxPhonemes, P_AW, 0, SAM_STRESS_NONE);
                    i++;
                } else {
                    addPhoneme(phonemes, count, maxPhonemes, P_AE, 0, SAM_STRESS_NONE);
                }
                break;
                
            case 'b':
                addPhoneme(phonemes, count, maxPhonemes, P_B, 0, SAM_STRESS_NONE);
                break;
                
            case 'c':
                if (next == 'h') {
                    addPhoneme(phonemes, count, maxPhonemes, P_CH, 0, SAM_STRESS_NONE);
                    i++;
                } else {
                    addPhoneme(phonemes, count, maxPhonemes, P_K, 0, SAM_STRESS_NONE);
                }
                break;
                
            case 'd':
                addPhoneme(phonemes, count, maxPhonemes, P_D, 0, SAM_STRESS_NONE);
                break;
                
            case 'e':
                if (i == len - 1) {
                    // Silent 'e' at end
                } else if (next == 'e') {
                    addPhoneme(phonemes, count, maxPhonemes, P_IY, 0, SAM_STRESS_NONE);
                    i++;
                } else {
                    addPhoneme(phonemes, count, maxPhonemes, P_EH, 0, SAM_STRESS_NONE);
                }
                break;
                
            case 'f':
                addPhoneme(phonemes, count, maxPhonemes, P_F, 0, SAM_STRESS_NONE);
                break;
                
            case 'g':
                addPhoneme(phonemes, count, maxPhonemes, P_G, 0, SAM_STRESS_NONE);
                break;
                
            case 'h':
                addPhoneme(phonemes, count, maxPhonemes, P_H, 0, SAM_STRESS_NONE);
                break;
                
            case 'i':
                addPhoneme(phonemes, count, maxPhonemes, P_IH, 0, SAM_STRESS_NONE);
                break;
                
            case 'j':
                addPhoneme(phonemes, count, maxPhonemes, P_J, 0, SAM_STRESS_NONE);
                break;
                
            case 'k':
                addPhoneme(phonemes, count, maxPhonemes, P_K, 0, SAM_STRESS_NONE);
                break;
                
            case 'l':
                addPhoneme(phonemes, count, maxPhonemes, P_L, 0, SAM_STRESS_NONE);
                break;
                
            case 'm':
                addPhoneme(phonemes, count, maxPhonemes, P_M, 0, SAM_STRESS_NONE);
                break;
                
            case 'n':
                if (next == 'g') {
                    addPhoneme(phonemes, count, maxPhonemes, P_NG, 0, SAM_STRESS_NONE);
                    i++;
                } else {
                    addPhoneme(phonemes, count, maxPhonemes, P_N, 0, SAM_STRESS_NONE);
                }
                break;
                
            case 'o':
                if (next == 'y') {
                    addPhoneme(phonemes, count, maxPhonemes, P_OY, 0, SAM_STRESS_NONE);
                    i++;
                } else if (next == 'w' || next == 'u') {
                    addPhoneme(phonemes, count, maxPhonemes, P_AW, 0, SAM_STRESS_NONE);
                    i++;
                } else {
                    addPhoneme(phonemes, count, maxPhonemes, P_AA, 0, SAM_STRESS_NONE);
                }
                break;
                
            case 'p':
                addPhoneme(phonemes, count, maxPhonemes, P_P, 0, SAM_STRESS_NONE);
                break;
                
            case 'r':
                addPhoneme(phonemes, count, maxPhonemes, P_R, 0, SAM_STRESS_NONE);
                break;
                
            case 's':
                if (next == 'h') {
                    addPhoneme(phonemes, count, maxPhonemes, P_SH, 0, SAM_STRESS_NONE);
                    i++;
                } else {
                    addPhoneme(phonemes, count, maxPhonemes, P_S, 0, SAM_STRESS_NONE);
                }
                break;
                
            case 't':
                if (next == 'h') {
                    addPhoneme(phonemes, count, maxPhonemes, P_TH, 0, SAM_STRESS_NONE);
                    i++;
                } else {
                    addPhoneme(phonemes, count, maxPhonemes, P_T, 0, SAM_STRESS_NONE);
                }
                break;
                
            case 'u':
                addPhoneme(phonemes, count, maxPhonemes, P_AH, 0, SAM_STRESS_NONE);
                break;
                
            case 'v':
                addPhoneme(phonemes, count, maxPhonemes, P_V, 0, SAM_STRESS_NONE);
                break;
                
            case 'w':
                addPhoneme(phonemes, count, maxPhonemes, P_W, 0, SAM_STRESS_NONE);
                break;
                
            case 'x':
                addPhoneme(phonemes, count, maxPhonemes, P_K, 0, SAM_STRESS_NONE);
                addPhoneme(phonemes, count, maxPhonemes, P_S, 0, SAM_STRESS_NONE);
                break;
                
            case 'y':
                addPhoneme(phonemes, count, maxPhonemes, P_Y, 0, SAM_STRESS_NONE);
                break;
                
            case 'z':
                addPhoneme(phonemes, count, maxPhonemes, P_Z, 0, SAM_STRESS_NONE);
                break;
        }
    }
    
    return count;
}
