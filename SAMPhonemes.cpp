// SAMPhonemes.cpp - Phoneme Table Implementation
// Modern formant values based on acoustic research, NOT C64 values

#include "SAMPhonemes.h"
#include <map>

namespace SAMFormantTables {
    
    // Helper function to create FormantSet
    inline FormantSet makeFormantSet(float f1, float f2, float f3,
                                     float a1, float a2, float a3,
                                     float bw1, float bw2, float bw3) {
        FormantSet fs;
        fs.f1 = f1; fs.f2 = f2; fs.f3 = f3;
        fs.a1 = a1; fs.a2 = a2; fs.a3 = a3;
        fs.bw1 = bw1; fs.bw2 = bw2; fs.bw3 = bw3;
        return fs;
    }
    
    // ========================================================================
    // Master Phoneme Lookup Table
    // Indexed by phoneme index (0-127)
    // ========================================================================
    
    const PhonemeFormantData PHONEME_TABLE[128] = {
        // [0-2] Silence & Pauses
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 100, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 300, 0, 0},
        
        // [3-9] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [10-20] Vowels
        {"IY", PhonemeType::VOWEL, makeFormantSet(270,2290,3010,1.0f,0.35f,0.20f,60,90,150), 120, 0, 1.0f},
        {"IH", PhonemeType::VOWEL, makeFormantSet(390,1990,2550,1.0f,0.40f,0.25f,70,100,160), 100, 0, 1.0f},
        {"EH", PhonemeType::VOWEL, makeFormantSet(530,1840,2480,1.0f,0.45f,0.28f,80,110,170), 110, 0, 1.0f},
        {"AE", PhonemeType::VOWEL, makeFormantSet(660,1720,2410,1.0f,0.50f,0.30f,90,120,180), 120, 0, 1.0f},
        {"AA", PhonemeType::VOWEL, makeFormantSet(730,1090,2440,1.0f,0.50f,0.30f,95,125,185), 130, 0, 1.0f},
        {"AO", PhonemeType::VOWEL, makeFormantSet(570,840,2410,1.0f,0.48f,0.28f,85,115,180), 140, 0, 1.0f},
        {"UH", PhonemeType::VOWEL, makeFormantSet(440,1020,2240,1.0f,0.42f,0.26f,75,105,170), 110, 0, 1.0f},
        {"UW", PhonemeType::VOWEL, makeFormantSet(300,870,2240,1.0f,0.40f,0.25f,70,100,165), 120, 0, 1.0f},
        {"ER", PhonemeType::VOWEL, makeFormantSet(490,1350,1690,1.0f,0.50f,0.35f,75,105,165), 130, 0, 1.0f},
        {"AH", PhonemeType::VOWEL, makeFormantSet(640,1190,2390,1.0f,0.45f,0.28f,85,115,175), 100, 0, 1.0f},
        {"AX", PhonemeType::VOWEL, makeFormantSet(500,1500,2500,0.90f,0.40f,0.25f,80,110,170), 80, 0, 1.0f},
        
        // [21-24] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [25-29] Diphthongs
        {"EY", PhonemeType::VOWEL, makeFormantSet(450,1900,2500,1.0f,0.43f,0.27f,75,105,175), 150, 0, 1.0f},
        {"AY", PhonemeType::VOWEL, makeFormantSet(700,1400,2400,1.0f,0.47f,0.29f,90,120,180), 150, 0, 1.0f},
        {"OY", PhonemeType::VOWEL, makeFormantSet(500,900,2300,1.0f,0.45f,0.28f,80,110,175), 150, 0, 1.0f},
        {"AW", PhonemeType::VOWEL, makeFormantSet(650,1000,2350,1.0f,0.46f,0.28f,85,115,175), 150, 0, 1.0f},
        {"OW", PhonemeType::VOWEL, makeFormantSet(450,900,2300,1.0f,0.44f,0.27f,80,110,170), 150, 0, 1.0f},
        
        // [30-39] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [40-45] Stops
        {"P", PhonemeType::CONSONANT_STOP, makeFormantSet(0,0,0,0,0,0,0,0,0), 80, 0.2f, 0.0f},
        {"B", PhonemeType::CONSONANT_STOP, makeFormantSet(0,0,0,0,0,0,0,0,0), 80, 0.1f, 0.3f},
        {"T", PhonemeType::CONSONANT_STOP, makeFormantSet(0,0,0,0,0,0,0,0,0), 70, 0.25f, 0.0f},
        {"D", PhonemeType::CONSONANT_STOP, makeFormantSet(0,0,0,0,0,0,0,0,0), 70, 0.15f, 0.3f},
        {"K", PhonemeType::CONSONANT_STOP, makeFormantSet(0,0,0,0,0,0,0,0,0), 90, 0.3f, 0.0f},
        {"G", PhonemeType::CONSONANT_STOP, makeFormantSet(0,0,0,0,0,0,0,0,0), 90, 0.2f, 0.3f},
        
        // [46-49] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [50-58] Fricatives
        {"F", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(200,1400,5000,0.3f,0.5f,0.8f,100,200,500), 100, 0.7f, 0.0f},
        {"V", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(200,1400,5000,0.4f,0.6f,0.7f,100,200,500), 90, 0.6f, 0.5f},
        {"TH", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(300,2000,6000,0.3f,0.5f,0.75f,120,250,600), 90, 0.65f, 0.0f},
        {"DH", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(300,2000,6000,0.4f,0.6f,0.7f,120,250,600), 80, 0.55f, 0.5f},
        {"S", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(400,2500,8000,0.3f,0.6f,0.9f,150,300,800), 110, 0.85f, 0.0f},
        {"Z", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(400,2500,8000,0.4f,0.65f,0.85f,150,300,800), 100, 0.75f, 0.5f},
        {"SH", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(300,1800,6000,0.35f,0.65f,0.85f,130,280,700), 120, 0.8f, 0.0f},
        {"ZH", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(300,1800,6000,0.4f,0.7f,0.8f,130,280,700), 110, 0.7f, 0.5f},
        {"H", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(500,1500,2500,0.3f,0.4f,0.5f,150,250,400), 70, 0.5f, 0.0f},
        
        // [59] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [60-61] Affricates
        {"CH", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(300,2000,7000,0.35f,0.65f,0.85f,140,280,750), 130, 0.75f, 0.0f},
        {"JH", PhonemeType::CONSONANT_FRICATIVE, makeFormantSet(300,2000,7000,0.4f,0.7f,0.8f,140,280,750), 120, 0.65f, 0.5f},
        
        // [62-69] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [70-72] Nasals
        {"M", PhonemeType::CONSONANT_NASAL, makeFormantSet(280,1300,2500,1.0f,0.4f,0.25f,60,100,150), 100, 0, 1.0f},
        {"N", PhonemeType::CONSONANT_NASAL, makeFormantSet(280,1700,2600,1.0f,0.42f,0.26f,60,100,150), 90, 0, 1.0f},
        {"NG", PhonemeType::CONSONANT_NASAL, makeFormantSet(280,2200,2900,1.0f,0.40f,0.25f,60,100,150), 100, 0, 1.0f},
        
        // [73-79] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [80-81] Liquids
        {"L", PhonemeType::CONSONANT_LIQUID, makeFormantSet(360,1300,2800,1.0f,0.45f,0.28f,70,110,160), 90, 0, 1.0f},
        {"R", PhonemeType::CONSONANT_LIQUID, makeFormantSet(420,1300,1700,1.0f,0.50f,0.35f,75,110,140), 90, 0, 1.0f},
        
        // [82-89] Reserved
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        
        // [90-91] Glides
        {"W", PhonemeType::CONSONANT_GLIDE, makeFormantSet(340,900,2300,1.0f,0.42f,0.26f,70,100,160), 80, 0, 1.0f},
        {"Y", PhonemeType::CONSONANT_GLIDE, makeFormantSet(310,2200,3000,1.0f,0.38f,0.22f,65,95,155), 80, 0, 1.0f},
        
        // [92-127] Reserved - all silence
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0},
        {" ", PhonemeType::SILENCE, makeFormantSet(0,0,0,0,0,0,0,0,0), 50, 0, 0}
    };
    
} // namespace SAMFormantTables

// ============================================================================
// Text-to-Phoneme Rules (Placeholder for full implementation)
// ============================================================================

namespace SAMTextRules {
    
    static std::map<String, String> g_dictionary;
    static bool g_initialized = false;
    
    void initializeRules() {
        if (g_initialized) return;
        
        // Initialize basic dictionary
        g_dictionary["THE"] = "DHAX";
        g_dictionary["A"] = "AX";
        g_dictionary["AND"] = "AEND";
        g_dictionary["IS"] = "IHZ";
        g_dictionary["ARE"] = "AAR";
        g_dictionary["WAS"] = "WAAZ";
        g_dictionary["WERE"] = "WER";
        g_dictionary["TO"] = "TUW";
        g_dictionary["OF"] = "AHV";
        g_dictionary["FOR"] = "FOHR";
        g_dictionary["WITH"] = "WIHTH";
        g_dictionary["FROM"] = "FRAHM";
        g_dictionary["HELLO"] = "HAXLOW";
        g_dictionary["WORLD"] = "WERLD";
        g_dictionary["ESP32"] = "IY EH S PIY THERTIY TUW";
        g_dictionary["AUDIO"] = "AODIYOW";
        g_dictionary["ENGINE"] = "EHNJHN";
        g_dictionary["SPEECH"] = "SPIYCH";
        g_dictionary["SYNTHESIS"] = "SIHNTHAXSIHS";
        
        g_initialized = true;
    }
    
    String textToPhonemes(const String& text) {
        initializeRules();
        
        String upper = text;
        upper.toUpperCase();
        
        if (g_dictionary.find(upper) != g_dictionary.end()) {
            return g_dictionary[upper];
        }
        
        return text;
    }
    
    bool lookupDictionary(const String& word, String& phonemes) {
        initializeRules();
        
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
    
} // namespace SAMTextRules