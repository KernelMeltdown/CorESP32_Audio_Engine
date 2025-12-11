// SAMCore.cpp - SAM Core Implementation

#include "SAMCore.h"
#include "SAMPhoneme.h"
#include "SAMFormant.h"
#include "SAMRenderer.h"

SAMCore::SAMCore() {
    reset();
}

void SAMCore::setSpeed(uint8_t speed) {
    params.speed = constrain(speed, SAM_SPEED_MIN, SAM_SPEED_MAX);
}

void SAMCore::setPitch(uint8_t pitch) {
    params.pitch = constrain(pitch, SAM_PITCH_MIN, SAM_PITCH_MAX);
}

void SAMCore::setThroat(uint8_t throat) {
    params.throat = constrain(throat, SAM_THROAT_MIN, SAM_THROAT_MAX);
}

void SAMCore::setMouth(uint8_t mouth) {
    params.mouth = constrain(mouth, SAM_MOUTH_MIN, SAM_MOUTH_MAX);
}

void SAMCore::setParams(const SAMVoiceParams& p) {
    params = p;
}

bool SAMCore::synthesize(const char* text) {
    SAM_PROFILE_START();
    
    if (!text || strlen(text) == 0) {
        return false;
    }
    
    // Reset state
    reset();
    
    // Convert text to phonemes
    state.phonemeCount = SAMPhoneme::textToPhonemes(
        text, 
        state.phonemes, 
        SAM_PHONEME_BUFFER
    );
    
    if (state.phonemeCount == 0) {
        #if SAM_DEBUG
        Serial.println("[SAM] No phonemes generated");
        #endif
        return false;
    }
    
    // Apply voice parameters to phonemes
    SAMFormant::applyVoiceParams(
        state.phonemes,
        state.phonemeCount,
        params
    );
    
    state.active = true;
    state.currentPhoneme = 0;
    state.sampleOffset = 0;
    
    SAM_PROFILE_END("synthesize");
    
    #if SAM_DEBUG
    Serial.printf("[SAM] Synthesized %d phonemes from: %s\n", 
                  state.phonemeCount, text);
    #endif
    
    return true;
}

size_t SAMCore::render(int16_t* buffer, size_t samples) {
    if (!state.active || !buffer || samples == 0) {
        return 0;
    }
    
    SAM_PROFILE_START();
    
    size_t samplesRendered = SAMRenderer::render(
        state.phonemes,
        state.phonemeCount,
        state.currentPhoneme,
        state.sampleOffset,
        formantState,
        params,
        buffer,
        samples
    );
    
    // Check if synthesis complete
    if (state.currentPhoneme >= state.phonemeCount) {
        state.active = false;
    }
    
    SAM_PROFILE_END("render");
    
    return samplesRendered;
}

void SAMCore::reset() {
    state.phonemeCount = 0;
    state.currentPhoneme = 0;
    state.sampleOffset = 0;
    state.active = false;
    formantState.reset();
}
