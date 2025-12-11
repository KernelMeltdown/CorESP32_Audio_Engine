/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  SAM PHONEME DEFINITIONS - Implementation                                    ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/
#include "SAMPhonemes.h"
#include <map>

using namespace SAMFormantTables;

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - VOWELS
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::SILENCE_DATA = {
    " ", "silence", PhonemeType::SILENCE,
    makeFormantSet(0, 0, 0, 0, 0, 0, 0, 0, 0),
    50, false
};

const PhonemeFormantData SAMFormantTables::IY_DATA = {
    "IY", "see", PhonemeType::VOWEL,
    makeFormantSet(270, 2290, 3010, 1.0f, 0.35f, 0.2f, 60, 90, 150),
    100, true
};

const PhonemeFormantData SAMFormantTables::IH_DATA = {
    "IH", "sit", PhonemeType::VOWEL,
    makeFormantSet(390, 1990, 2550, 1.0f, 0.4f, 0.25f, 70, 100, 160),
    80, true
};

const PhonemeFormantData SAMFormantTables::EH_DATA = {
    "EH", "bed", PhonemeType::VOWEL,
    makeFormantSet(530, 1840, 2480, 1.0f, 0.45f, 0.28f, 80, 110, 170),
    90, true
};

const PhonemeFormantData SAMFormantTables::AE_DATA = {
    "AE", "cat", PhonemeType::VOWEL,
    makeFormantSet(660, 1720, 2410, 1.0f, 0.5f, 0.3f, 90, 120, 180),
    110, true
};

const PhonemeFormantData SAMFormantTables::AH_DATA = {
    "AH", "but", PhonemeType::VOWEL,
    makeFormantSet(640, 1190, 2390, 1.0f, 0.45f, 0.28f, 85, 115, 175),
    95, true
};

const PhonemeFormantData SAMFormantTables::AX_DATA = {
    "AX", "about", PhonemeType::VOWEL,
    makeFormantSet(500, 1500, 2500, 0.9f, 0.4f, 0.25f, 80, 110, 170),
    70, true
};

const PhonemeFormantData SAMFormantTables::ER_DATA = {
    "ER", "bird", PhonemeType::VOWEL,
    makeFormantSet(490, 1350, 1690, 1.0f, 0.5f, 0.35f, 75, 105, 165),
    100, true
};

const PhonemeFormantData SAMFormantTables::AA_DATA = {
    "AA", "hot", PhonemeType::VOWEL,
    makeFormantSet(730, 1090, 2440, 1.0f, 0.5f, 0.3f, 95, 125, 185),
    120, true
};

const PhonemeFormantData SAMFormantTables::AO_DATA = {
    "AO", "law", PhonemeType::VOWEL,
    makeFormantSet(570, 840, 2410, 1.0f, 0.48f, 0.28f, 85, 115, 180),
    115, true
};

const PhonemeFormantData SAMFormantTables::UH_DATA = {
    "UH", "put", PhonemeType::VOWEL,
    makeFormantSet(440, 1020, 2240, 1.0f, 0.42f, 0.26f, 75, 105, 170),
    85, true
};

const PhonemeFormantData SAMFormantTables::UW_DATA = {
    "UW", "food", PhonemeType::VOWEL,
    makeFormantSet(300, 870, 2240, 1.0f, 0.4f, 0.25f, 70, 100, 165),
    110, true
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - DIPHTHONGS
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::EY_DATA = {
    "EY", "day", PhonemeType::DIPHTHONG,
    makeFormantSet(450, 1900, 2500, 1.0f, 0.43f, 0.27f, 75, 105, 175),
    130, true
};

const PhonemeFormantData SAMFormantTables::AY_DATA = {
    "AY", "my", PhonemeType::DIPHTHONG,
    makeFormantSet(700, 1400, 2400, 1.0f, 0.47f, 0.29f, 90, 120, 180),
    140, true
};

const PhonemeFormantData SAMFormantTables::OY_DATA = {
    "OY", "boy", PhonemeType::DIPHTHONG,
    makeFormantSet(500, 900, 2300, 1.0f, 0.45f, 0.28f, 80, 110, 175),
    135, true
};

const PhonemeFormantData SAMFormantTables::AW_DATA = {
    "AW", "how", PhonemeType::DIPHTHONG,
    makeFormantSet(650, 1000, 2350, 1.0f, 0.46f, 0.28f, 85, 115, 175),
    140, true
};

const PhonemeFormantData SAMFormantTables::OW_DATA = {
    "OW", "go", PhonemeType::DIPHTHONG,
    makeFormantSet(450, 900, 2300, 1.0f, 0.44f, 0.27f, 80, 110, 170),
    130, true
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - STOPS
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::P_DATA = {
    "P", "put", PhonemeType::STOP,
    makeFormantSet(0, 0, 0, 0, 0, 0, 0, 0, 0),
    60, false
};

const PhonemeFormantData SAMFormantTables::B_DATA = {
    "B", "but", PhonemeType::STOP,
    makeFormantSet(0, 0, 0, 0, 0, 0, 0, 0, 0),
    60, true
};

const PhonemeFormantData SAMFormantTables::T_DATA = {
    "T", "top", PhonemeType::STOP,
    makeFormantSet(0, 0, 0, 0, 0, 0, 0, 0, 0),
    50, false
};

const PhonemeFormantData SAMFormantTables::D_DATA = {
    "D", "dog", PhonemeType::STOP,
    makeFormantSet(0, 0, 0, 0, 0, 0, 0, 0, 0),
    50, true
};

const PhonemeFormantData SAMFormantTables::K_DATA = {
    "K", "cat", PhonemeType::STOP,
    makeFormantSet(0, 0, 0, 0, 0, 0, 0, 0, 0),
    65, false
};

const PhonemeFormantData SAMFormantTables::G_DATA = {
    "G", "got", PhonemeType::STOP,
    makeFormantSet(0, 0, 0, 0, 0, 0, 0, 0, 0),
    65, true
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - FRICATIVES
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::F_DATA = {
    "F", "fan", PhonemeType::FRICATIVE,
    makeFormantSet(200, 1400, 5000, 0.3f, 0.5f, 0.8f, 100, 200, 500),
    90, false
};

const PhonemeFormantData SAMFormantTables::V_DATA = {
    "V", "van", PhonemeType::FRICATIVE,
    makeFormantSet(200, 1400, 5000, 0.4f, 0.6f, 0.7f, 100, 200, 500),
    90, true
};

const PhonemeFormantData SAMFormantTables::TH_DATA = {
    "TH", "thin", PhonemeType::FRICATIVE,
    makeFormantSet(300, 2000, 6000, 0.3f, 0.5f, 0.75f, 120, 250, 600),
    85, false
};

const PhonemeFormantData SAMFormantTables::DH_DATA = {
    "DH", "then", PhonemeType::FRICATIVE,
    makeFormantSet(300, 2000, 6000, 0.4f, 0.6f, 0.7f, 120, 250, 600),
    85, true
};

const PhonemeFormantData SAMFormantTables::S_DATA = {
    "S", "sit", PhonemeType::FRICATIVE,
    makeFormantSet(400, 2500, 8000, 0.3f, 0.6f, 0.9f, 150, 300, 800),
    95, false
};

const PhonemeFormantData SAMFormantTables::Z_DATA = {
    "Z", "zoo", PhonemeType::FRICATIVE,
    makeFormantSet(400, 2500, 8000, 0.4f, 0.65f, 0.85f, 150, 300, 800),
    95, true
};

const PhonemeFormantData SAMFormantTables::SH_DATA = {
    "SH", "shop", PhonemeType::FRICATIVE,
    makeFormantSet(300, 1800, 6000, 0.35f, 0.65f, 0.85f, 130, 280, 700),
    100, false
};

const PhonemeFormantData SAMFormantTables::ZH_DATA = {
    "ZH", "measure", PhonemeType::FRICATIVE,
    makeFormantSet(300, 1800, 6000, 0.4f, 0.7f, 0.8f, 130, 280, 700),
    100, true
};

const PhonemeFormantData SAMFormantTables::H_DATA = {
    "H", "hot", PhonemeType::FRICATIVE,
    makeFormantSet(500, 1500, 2500, 0.3f, 0.4f, 0.5f, 150, 250, 400),
    70, false
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - AFFRICATES
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::CH_DATA = {
    "CH", "church", PhonemeType::AFFRICATE,
    makeFormantSet(300, 2000, 7000, 0.35f, 0.65f, 0.85f, 140, 280, 750),
    105, false
};

const PhonemeFormantData SAMFormantTables::JH_DATA = {
    "JH", "judge", PhonemeType::AFFRICATE,
    makeFormantSet(300, 2000, 7000, 0.4f, 0.7f, 0.8f, 140, 280, 750),
    105, true
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - NASALS
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::M_DATA = {
    "M", "man", PhonemeType::NASAL,
    makeFormantSet(280, 1300, 2500, 1.0f, 0.4f, 0.25f, 60, 100, 150),
    85, true
};

const PhonemeFormantData SAMFormantTables::N_DATA = {
    "N", "not", PhonemeType::NASAL,
    makeFormantSet(280, 1700, 2600, 1.0f, 0.42f, 0.26f, 60, 100, 150),
    85, true
};

const PhonemeFormantData SAMFormantTables::NG_DATA = {
    "NG", "sing", PhonemeType::NASAL,
    makeFormantSet(280, 2200, 2900, 1.0f, 0.4f, 0.25f, 60, 100, 150),
    90, true
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - LIQUIDS
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::L_DATA = {
    "L", "let", PhonemeType::LIQUID,
    makeFormantSet(360, 1300, 2800, 1.0f, 0.45f, 0.28f, 70, 110, 160),
    75, true
};

const PhonemeFormantData SAMFormantTables::R_DATA = {
    "R", "red", PhonemeType::LIQUID,
    makeFormantSet(420, 1300, 1700, 1.0f, 0.5f, 0.35f, 75, 110, 140),
    80, true
};

// ═══════════════════════════════════════════════════════════════════════════
// Phoneme Formant Data - GLIDES
// ═══════════════════════════════════════════════════════════════════════════

const PhonemeFormantData SAMFormantTables::W_DATA = {
    "W", "wet", PhonemeType::GLIDE,
    makeFormantSet(340, 900, 2300, 1.0f, 0.42f, 0.26f, 70, 100, 160),
    75, true
};

const PhonemeFormantData SAMFormantTables::Y_DATA = {
    "Y", "yes", PhonemeType::GLIDE,
    makeFormantSet(310, 2200, 3000, 1.0f, 0.38f, 0.22f, 65, 95, 155),
    70, true
};

// ═══════════════════════════════════════════════════════════════════════════
// Text-to-Phoneme Dictionary
// ═══════════════════════════════════════════════════════════════════════════

namespace SAMTextRules {
    static std::map<String, String> g_dictionary;
    static bool g_initialized = false;
    
    void initializeRules() {
        if (g_initialized) return;
        
        // Common words dictionary
        g_dictionary["THE"] = "DHAX";
        g_dictionary["A"] = "AX";
        g_dictionary["AN"] = "AEN";
        g_dictionary["AND"] = "AEND";
        g_dictionary["IS"] = "IHZ";
        g_dictionary["ARE"] = "AAR";
        g_dictionary["WAS"] = "WAAZ";
        g_dictionary["WERE"] = "WER";
        g_dictionary["HELLO"] = "HAXLOW";
        g_dictionary["WORLD"] = "WERLD";
        g_dictionary["ESP32"] = "IYESPIYTHEERTIYTUUW";
        g_dictionary["SAM"] = "SAEM";
        
        g_initialized = true;
        Serial.println("[SAM] Phoneme dictionary initialized");
    }
    
    String textToPhonemes(const String& text) {
        initializeRules();
        
        String result = "";
        String word = "";
        String upper = text;
        upper.toUpperCase();
        
        for (int i = 0; i < upper.length(); i++) {
            char c = upper.charAt(i);
            
            if (isAlpha(c) || isDigit(c)) {
                word += c;
            } else {
                if (word.length() > 0) {
                    String phonemes;
                    if (lookupDictionary(word, phonemes)) {
                        result += phonemes;
                    } else {
                        // Simple letter-to-phoneme fallback
                        result += word;
                    }
                    word = "";
                }
                if (c == ' ') result += " ";
            }
        }
        
        // Process last word
        if (word.length() > 0) {
            String phonemes;
            if (lookupDictionary(word, phonemes)) {
                result += phonemes;
            } else {
                result += word;
            }
        }
        
        return result;
    }
    
    bool lookupDictionary(const String& word, String& phonemes) {
        String upper = word;
        upper.toUpperCase();
        
        if (g_dictionary.find(upper) != g_dictionary.end()) {
            phonemes = g_dictionary[upper];
            return true;
        }
        return false;
    }
    
    void addDictionaryEntry(const String& word, const String& phonemes) {
        String upper = word;
        upper.toUpperCase();
        g_dictionary[upper] = phonemes;
    }
}