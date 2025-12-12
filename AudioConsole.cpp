/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CONSOLE - Implementation v1.9                                         ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "AudioConsole.h"
#include "AudioEngine.h"
#include "AudioProfile.h"
#include "AudioFilesystem.h"
#include "AudioCodecManager.h"
#include <ArduinoJson.h>


AudioConsole::AudioConsole()
  : audio(nullptr), profileManager(nullptr), filesystem(nullptr), codecManager(nullptr) {
  for (int i = 0; i < MAX_SCHEDULED_NOTES; i++) {
    scheduledNotes[i].active = false;
  }
}

void AudioConsole::init(AudioEngine* audioEngine, AudioProfile* profMgr,
                        AudioFilesystem* fs, AudioCodecManager* codecMgr) {
  audio = audioEngine;
  profileManager = profMgr;
  filesystem = fs;
  codecManager = codecMgr;

  Serial.println();
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║            ESP32 AUDIO OS v1.9.0                       ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
  Serial.println();
  Serial.println(F("Type 'audio help' for commands"));
  Serial.println();
  Serial.print(CONSOLE_PROMPT);
}

void AudioConsole::update() {
  updateScheduledNotes();

  if (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (cmdBuffer.length() > 0) {
        Serial.println();
        processCommand(cmdBuffer);
        cmdBuffer = "";
        Serial.print(CONSOLE_PROMPT);
      }
    } else if (c == '\b' || c == 127) {
      if (cmdBuffer.length() > 0) {
        cmdBuffer.remove(cmdBuffer.length() - 1);
        Serial.write('\b');
        Serial.write(' ');
        Serial.write('\b');
      }
    } else if (c >= 32 && c <= 126) {
      if (cmdBuffer.length() < CONSOLE_MAX_CMD_LEN) {
        cmdBuffer += c;
        Serial.write(c);
      }
    }
  }
}

// ============================================================================
// COMMAND PARSING UTILITIES
// ============================================================================

String AudioConsole::getArg(const String& input, int index) {
  int start = 0;
  int count = 0;

  for (int i = 0; i <= input.length(); i++) {
    if (i == input.length() || input[i] == ' ') {
      if (i > start) {
        if (count == index) {
          return input.substring(start, i);
        }
        count++;
      }
      start = i + 1;
    }
  }

  return "";
}

int AudioConsole::countArgs(const String& input) {
  int count = 0;
  bool inWord = false;

  for (int i = 0; i < input.length(); i++) {
    if (input[i] != ' ') {
      if (!inWord) {
        count++;
        inWord = true;
      }
    } else {
      inWord = false;
    }
  }

  return count;
}

// ============================================================================
// COMMAND DISPATCHER
// ============================================================================

void AudioConsole::processCommand(String cmd) {
  cmd.trim();

  if (cmd.length() == 0) return;

  if (!cmd.startsWith("audio")) {
    Serial.println(F("[ERROR] Commands must start with 'audio'"));
    Serial.println(F("[HINT] Try: audio help"));
    return;
  }

  String args = cmd.substring(5);
  args.trim();

  if (args.length() == 0) {
    cmdHelp("");
    return;
  }

  String command = getArg(args, 0);
  command.toLowerCase();
  String remaining = args.substring(command.length());
  remaining.trim();

  // Command dispatch
  if (command == "help" || command == "?") {
    cmdHelp(remaining);
  } else if (command == "info") {
    cmdInfo(remaining);
  } else if (command == "status") {
    cmdStatus(remaining);
  } else if (command == "version") {
    cmdVersion(remaining);
  } else if (command == "play") {
    cmdPlay(remaining);
  } else if (command == "stop") {
    cmdStop(remaining);
  } else if (command == "volume" || command == "vol") {
    cmdVolume(remaining);
  } else if (command == "note") {
    cmdNote(remaining);
  } else if (command == "waveform" || command == "wave") {
    cmdWaveform(remaining);
  } else if (command == "eq") {
    cmdEQ(remaining);
  } else if (command == "filter") {
    cmdFilter(remaining);
  } else if (command == "reverb") {
    cmdReverb(remaining);
  } else if (command == "lfo") {
    cmdLFO(remaining);  // NEW!
  } else if (command == "delay") {
    cmdDelay(remaining);
  } else if (command == "profile") {
    cmdProfile(remaining);
  } else if (command == "mode") {
    cmdMode(remaining);
  } else if (command == "hw" || command == "hardware") {
    cmdHardware(remaining);
  } else if (command == "config") {
    cmdConfig(remaining);
  } else if (command == "codec") {
    cmdCodec(remaining);
  } else if (command == "list" || command == "ls") {
    cmdList(remaining);
  } else if (command == "test") {
    cmdTest(remaining);
  } else if (command == "reset") {
    cmdReset(remaining);
  } else if (command == "reboot") {
    cmdReboot(remaining);
  } else {
    Serial.printf("[ERROR] Unknown command: %s\n", command.c_str());
    Serial.println(F("[HINT] Type 'audio help' for available commands"));
  }
}

// ============================================================================
// PLAYBACK COMMANDS
// ============================================================================

void AudioConsole::cmdPlay(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println(F("ERROR: Usage: audio play <melody_name>"));
    Serial.println(F("HINT: Try 'audio play tetris' or 'audio list /melodies'"));
    return;
  }

  // Build melody path
  String path = String(PATH_MELODIES) + "/" + args;
  if (!path.endsWith(".json")) {
    path += ".json";
  }

  if (!loadAndPlayMelody(path.c_str())) {
    Serial.printf("ERROR: Could not load melody: %s\n", path.c_str());
    Serial.println(F("HINT: Use 'audio list /melodies' to see available melodies"));
  }
}

void AudioConsole::cmdStop(String args) {
  audio->stopMelody();
  audio->allNotesOff();
  Serial.println(F("[AUDIO] Stopped"));
}

void AudioConsole::cmdVolume(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.printf("Volume: %d/255\n", audio->getVolume());
    return;
  }

  int vol = args.toInt();
  if (vol < 0 || vol > 255) {
    Serial.println(F("[ERROR] Volume must be 0-255"));
    return;
  }

  audio->setVolume(vol);
  Serial.printf("[OK] Volume: %d/255\n", vol);
}

void AudioConsole::cmdNote(String args) {
  if (countArgs(args) < 1) {
    Serial.println(F("[ERROR] Usage: audio note <0-127> [duration_ms]"));
    return;
  }

  int note = getArg(args, 0).toInt();
  if (note < 0 || note > 127) {
    Serial.println(F("[ERROR] Note must be 0-127"));
    return;
  }

  int duration = 1000;
  if (countArgs(args) >= 2) {
    duration = getArg(args, 1).toInt();
    duration = constrain(duration, 10, 10000);
  }

  Serial.printf("[NOTE] Playing MIDI note %d for %d ms\n", note, duration);
  audio->noteOn(note, 127);
  scheduleNoteOff(note, duration);
}

void AudioConsole::cmdWaveform(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.printf("Current waveform: %s\n", audio->getWaveformName());
    Serial.println();
    Serial.println(F("Available waveforms:"));
    Serial.println(F("  sine      - Pure sine wave (smooth)"));
    Serial.println(F("  square    - Square wave (8-bit retro)"));
    Serial.println(F("  sawtooth  - Sawtooth wave (bright)"));
    Serial.println(F("  triangle  - Triangle wave (mellow)"));
    Serial.println(F("  noise     - White noise"));
    Serial.println();
    return;
  }

  args.toLowerCase();

  WaveformType waveform;
  if (args == "sine") {
    waveform = WAVE_SINE;
  } else if (args == "square") {
    waveform = WAVE_SQUARE;
  } else if (args == "sawtooth" || args == "saw") {
    waveform = WAVE_SAWTOOTH;
  } else if (args == "triangle" || args == "tri") {
    waveform = WAVE_TRIANGLE;
  } else if (args == "noise") {
    waveform = WAVE_NOISE;
  } else {
    Serial.println(F("[ERROR] Unknown waveform"));
    Serial.println(F("Use: sine, square, sawtooth, triangle, noise"));
    return;
  }

  audio->setWaveform(waveform);
  Serial.printf("[OK] Waveform: %s\n", audio->getWaveformName());
}

// ============================================================================
// STATE-VARIABLE FILTER COMMAND
// ============================================================================

// ============================================================================
// STATE-VARIABLE FILTER COMMAND (FIXED!)
// ============================================================================

void AudioConsole::cmdFilter(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println();
    Serial.println(F("State-Variable Filter Settings:"));
    Serial.printf("  Enabled:      %s\n", audio->getFilterEnabled() ? "Yes" : "No");
    Serial.printf("  Type:         %s\n", audio->getFilterTypeName());
    Serial.printf("  Cutoff:       %.1f Hz\n", audio->getFilterCutoff());
    Serial.printf("  Resonance:    %.2f\n", audio->getFilterResonance());
    Serial.println();
    Serial.println(F("Usage:"));
    Serial.println(F("  audio filter on|off"));
    Serial.println(F("  audio filter <lowpass|highpass|bandpass>"));
    Serial.println(F("  audio filter cutoff <20-20000>"));
    Serial.println(F("  audio filter resonance <0.0-1.0>"));
    Serial.println();
    Serial.println(F("Examples:"));
    Serial.println(F("  audio filter on"));
    Serial.println(F("  audio filter lowpass"));
    Serial.println(F("  audio filter cutoff 800"));
    Serial.println(F("  audio filter resonance 0.7"));
    Serial.println();
    Serial.println(F("TIP: Try lowpass + resonance with square wave!"));
    Serial.println();
    return;
  }

  String param = getArg(args, 0);
  param.toLowerCase();

  if (param == "on") {
    audio->setFilterEnabled(true);
    Serial.println(F("[OK] Filter enabled"));
    return;

  } else if (param == "off") {
    audio->setFilterEnabled(false);
    Serial.println(F("[OK] Filter disabled"));
    return;

  } else if (param == "lowpass" || param == "lp") {
    audio->setFilterType(FILTER_LOWPASS);
    Serial.println(F("[OK] Filter type: Lowpass"));
    if (!audio->getFilterEnabled()) {
      Serial.println(F("[HINT] Filter is disabled - use 'audio filter on' to enable"));
    }
    return;

  } else if (param == "highpass" || param == "hp") {
    audio->setFilterType(FILTER_HIGHPASS);
    Serial.println(F("[OK] Filter type: Highpass"));
    if (!audio->getFilterEnabled()) {
      Serial.println(F("[HINT] Filter is disabled - use 'audio filter on' to enable"));
    }
    return;

  } else if (param == "bandpass" || param == "bp") {
    audio->setFilterType(FILTER_BANDPASS);
    Serial.println(F("[OK] Filter type: Bandpass"));
    if (!audio->getFilterEnabled()) {
      Serial.println(F("[HINT] Filter is disabled - use 'audio filter on' to enable"));
    }
    return;

  } else if (param == "cutoff" || param == "freq") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio filter cutoff <20-20000>"));
      return;
    }

    float cutoff = getArg(args, 1).toFloat();
    if (cutoff < 20.0f || cutoff > 20000.0f) {
      Serial.println(F("[ERROR] Cutoff must be 20-20000 Hz"));
      return;
    }

    audio->setFilterCutoff(cutoff);
    Serial.printf("[OK] Filter cutoff: %.1f Hz\n", cutoff);

    if (!audio->getFilterEnabled()) {
      Serial.println(F("[HINT] Filter is disabled - use 'audio filter on' to enable"));
    }
    return;

  } else if (param == "resonance" || param == "res" || param == "q") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio filter resonance <0.0-1.0>"));
      return;
    }

    float resonance = getArg(args, 1).toFloat();
    if (resonance < 0.0f || resonance > 1.0f) {
      Serial.println(F("[ERROR] Resonance must be 0.0-1.0"));
      return;
    }

    audio->setFilterResonance(resonance);
    Serial.printf("[OK] Filter resonance: %.2f\n", resonance);

    if (!audio->getFilterEnabled()) {
      Serial.println(F("[HINT] Filter is disabled - use 'audio filter on' to enable"));
    }
    return;

  } else {
    Serial.println(F("[ERROR] Unknown parameter"));
    Serial.println(F("Use: on, off, lowpass, highpass, bandpass, cutoff, resonance"));
  }
}

// ============================================================================
// DELAY COMMAND
// ============================================================================

void AudioConsole::cmdDelay(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println();
    Serial.println(F("Delay/Echo Settings:"));
    Serial.printf("  Enabled:      %s\n", audio->getDelayEnabled() ? "Yes" : "No");
    Serial.printf("  Time:         %u ms\n", audio->getDelayTime());
    Serial.printf("  Feedback:     %u%%\n", audio->getDelayFeedback());
    Serial.printf("  Mix:          %u%% (wet)\n", audio->getDelayMix());
    Serial.println();
    Serial.println(F("Usage:"));
    Serial.println(F("  audio delay on|off"));
    Serial.println(F("  audio delay time <10-1000>"));
    Serial.println(F("  audio delay feedback <0-90>"));
    Serial.println(F("  audio delay mix <0-100>"));
    Serial.println();
    return;
  }

  String param = getArg(args, 0);
  param.toLowerCase();

  if (param == "on") {
    audio->setDelayEnabled(true);
    Serial.println(F("[OK] Delay enabled"));

  } else if (param == "off") {
    audio->setDelayEnabled(false);
    Serial.println(F("[OK] Delay disabled"));

  } else if (param == "time") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio delay time <10-1000>"));
      return;
    }

    int time = getArg(args, 1).toInt();
    if (time < 10 || time > 1000) {
      Serial.println(F("[ERROR] Delay time must be 10-1000 ms"));
      return;
    }

    audio->setDelayTime(time);
    Serial.printf("[OK] Delay time: %d ms\n", time);

  } else if (param == "feedback" || param == "fb") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio delay feedback <0-90>"));
      return;
    }

    int feedback = getArg(args, 1).toInt();
    if (feedback < 0 || feedback > 90) {
      Serial.println(F("[ERROR] Feedback must be 0-90%"));
      return;
    }

    audio->setDelayFeedback(feedback);
    Serial.printf("[OK] Delay feedback: %d%%\n", feedback);

  } else if (param == "mix") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio delay mix <0-100>"));
      return;
    }

    int mix = getArg(args, 1).toInt();
    if (mix < 0 || mix > 100) {
      Serial.println(F("[ERROR] Mix must be 0-100%"));
      return;
    }

    audio->setDelayMix(mix);
    Serial.printf("[OK] Delay mix: %d%% wet\n", mix);

  } else {
    Serial.println(F("[ERROR] Unknown parameter"));
    Serial.println(F("Use: on, off, time, feedback, mix"));
  }
}


// ============================================================================
// EQ COMMAND
// ============================================================================

void AudioConsole::cmdEQ(String args) {
  args.trim();

  if (args.length() == 0) {
    int8_t bass, mid, treble;
    audio->getEQ(bass, mid, treble);
    Serial.println();
    Serial.println(F("Biquad EQ Settings:"));
    Serial.printf("  Enabled:      %s\n", audio->getEQEnabled() ? "Yes" : "No");
    Serial.printf("  Bass:         %+d dB (120 Hz)\n", bass);
    Serial.printf("  Mid:          %+d dB (1000 Hz)\n", mid);
    Serial.printf("  Treble:       %+d dB (8000 Hz)\n", treble);
    Serial.println();
    Serial.println(F("Usage:"));
    Serial.println(F("  audio eq on|off"));
    Serial.println(F("  audio eq <bass|mid|treble> <-12..+12>"));
    Serial.println();
    Serial.println(F("Examples:"));
    Serial.println(F("  audio eq on"));
    Serial.println(F("  audio eq bass +6"));
    Serial.println(F("  audio eq treble -3"));
    Serial.println();
    return;
  }

  String param = getArg(args, 0);
  param.toLowerCase();

  if (param == "on") {
    audio->setEQEnabled(true);
    Serial.println(F("[OK] Biquad EQ enabled"));
    return;

  } else if (param == "off") {
    audio->setEQEnabled(false);
    Serial.println(F("[OK] Biquad EQ disabled"));
    return;
  }

  if (countArgs(args) < 2) {
    Serial.println(F("[ERROR] Usage: audio eq <bass|mid|treble> <-12 to +12>"));
    return;
  }

  String band = param;
  int value = getArg(args, 1).toInt();
  value = constrain(value, -12, 12);

  int8_t bass, mid, treble;
  audio->getEQ(bass, mid, treble);

  if (band == "bass" || band == "b") {
    bass = value;
  } else if (band == "mid" || band == "m") {
    mid = value;
  } else if (band == "treble" || band == "t") {
    treble = value;
  } else {
    Serial.println(F("[ERROR] Invalid band: bass|mid|treble"));
    return;
  }

  audio->setEQ(bass, mid, treble);
  Serial.printf("[OK] EQ %s: %+d dB\n", band.c_str(), value);

  if (!audio->getEQEnabled()) {
    Serial.println(F("[HINT] EQ is disabled - use 'audio eq on' to enable"));
  }
}

// ============================================================================
// REVERB COMMAND
// ============================================================================

void AudioConsole::cmdReverb(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println();
    Serial.println(F("Schroeder Reverb Settings:"));
    Serial.printf("  Enabled:      %s\n", audio->getReverbEnabled() ? "Yes" : "No");
    Serial.printf("  Room Size:    %.2f (0.0-1.0)\n", audio->getReverbRoomSize());
    Serial.printf("  Damping:      %.2f (0.0-1.0)\n", audio->getReverbDamping());
    Serial.printf("  Wet Mix:      %.2f (0.0-1.0)\n", audio->getReverbWet());
    Serial.println();
    Serial.println(F("Usage:"));
    Serial.println(F("  audio reverb on|off"));
    Serial.println(F("  audio reverb room <0.0-1.0>"));
    Serial.println(F("  audio reverb damping <0.0-1.0>"));
    Serial.println(F("  audio reverb wet <0.0-1.0>"));
    Serial.println();
    Serial.println(F("Examples:"));
    Serial.println(F("  audio reverb on"));
    Serial.println(F("  audio reverb room 0.7       # Large room"));
    Serial.println(F("  audio reverb damping 0.5    # Medium damping"));
    Serial.println(F("  audio reverb wet 0.4        # 40% reverb"));
    Serial.println();
    return;
  }

  String param = getArg(args, 0);
  param.toLowerCase();

  if (param == "on") {
    audio->setReverbEnabled(true);
    Serial.println(F("[OK] Reverb enabled"));
    return;

  } else if (param == "off") {
    audio->setReverbEnabled(false);
    Serial.println(F("[OK] Reverb disabled"));
    return;

  } else if (param == "room" || param == "size" || param == "roomsize") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio reverb room <0.0-1.0>"));
      return;
    }

    float size = getArg(args, 1).toFloat();
    if (size < 0.0f || size > 1.0f) {
      Serial.println(F("[ERROR] Room size must be 0.0-1.0"));
      return;
    }

    audio->setReverbRoomSize(size);
    Serial.printf("[OK] Reverb room size: %.2f\n", size);

    if (!audio->getReverbEnabled()) {
      Serial.println(F("[HINT] Reverb is disabled - use 'audio reverb on' to enable"));
    }
    return;

  } else if (param == "damping" || param == "damp") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio reverb damping <0.0-1.0>"));
      return;
    }

    float damping = getArg(args, 1).toFloat();
    if (damping < 0.0f || damping > 1.0f) {
      Serial.println(F("[ERROR] Damping must be 0.0-1.0"));
      return;
    }

    audio->setReverbDamping(damping);
    Serial.printf("[OK] Reverb damping: %.2f\n", damping);

    if (!audio->getReverbEnabled()) {
      Serial.println(F("[HINT] Reverb is disabled - use 'audio reverb on' to enable"));
    }
    return;

  } else if (param == "wet" || param == "mix") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio reverb wet <0.0-1.0>"));
      return;
    }

    float wet = getArg(args, 1).toFloat();
    if (wet < 0.0f || wet > 1.0f) {
      Serial.println(F("[ERROR] Wet mix must be 0.0-1.0"));
      return;
    }

    audio->setReverbWet(wet);
    Serial.printf("[OK] Reverb wet: %.2f\n", wet);

    if (!audio->getReverbEnabled()) {
      Serial.println(F("[HINT] Reverb is disabled - use 'audio reverb on' to enable"));
    }
    return;

  } else {
    Serial.println(F("[ERROR] Unknown parameter"));
    Serial.println(F("Use: on, off, room, damping, wet"));
  }
}

// ============================================================================
// LFO COMMAND (NEW!)
// ============================================================================

void AudioConsole::cmdLFO(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println();
    Serial.println(F("LFO Modulation Settings:"));
    Serial.printf("  Enabled:      %s\n", audio->getLFOEnabled() ? "Yes" : "No");
    Serial.printf("  Vibrato:      %s\n", audio->getLFOVibratoEnabled() ? "On" : "Off");
    Serial.printf("  Tremolo:      %s\n", audio->getLFOTremoloEnabled() ? "On" : "Off");
    Serial.printf("  Rate:         %.2f Hz\n", audio->getLFORate());
    Serial.printf("  Depth:        %.1f%%\n", audio->getLFODepth());
    Serial.println();
    Serial.println(F("Usage:"));
    Serial.println(F("  audio lfo on|off"));
    Serial.println(F("  audio lfo vibrato on|off"));
    Serial.println(F("  audio lfo tremolo on|off"));
    Serial.println(F("  audio lfo rate <0.1-20.0>"));
    Serial.println(F("  audio lfo depth <0-100>"));
    Serial.println();
    Serial.println(F("Examples:"));
    Serial.println(F("  audio lfo on"));
    Serial.println(F("  audio lfo vibrato on"));
    Serial.println(F("  audio lfo rate 5.0         # 5 Hz modulation"));
    Serial.println(F("  audio lfo depth 30         # 30% intensity"));
    Serial.println();
    Serial.println(F("TIP: Vibrato = pitch wobble, Tremolo = volume pulse"));
    Serial.println();
    return;
  }

  String param = getArg(args, 0);
  param.toLowerCase();

  if (param == "on") {
    audio->setLFOEnabled(true);
    Serial.println(F("[OK] LFO enabled"));
    return;

  } else if (param == "off") {
    audio->setLFOEnabled(false);
    Serial.println(F("[OK] LFO disabled"));
    return;

  } else if (param == "vibrato" || param == "vib") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio lfo vibrato on|off"));
      return;
    }

    String value = getArg(args, 1);
    value.toLowerCase();

    if (value == "on") {
      audio->setLFOVibratoEnabled(true);
      Serial.println(F("[OK] LFO Vibrato enabled"));
      if (!audio->getLFOEnabled()) {
        Serial.println(F("[HINT] LFO is disabled - use 'audio lfo on' to enable"));
      }
    } else if (value == "off") {
      audio->setLFOVibratoEnabled(false);
      Serial.println(F("[OK] LFO Vibrato disabled"));
    } else {
      Serial.println(F("[ERROR] Use: on|off"));
    }
    return;

  } else if (param == "tremolo" || param == "trem") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio lfo tremolo on|off"));
      return;
    }

    String value = getArg(args, 1);
    value.toLowerCase();

    if (value == "on") {
      audio->setLFOTremoloEnabled(true);
      Serial.println(F("[OK] LFO Tremolo enabled"));
      if (!audio->getLFOEnabled()) {
        Serial.println(F("[HINT] LFO is disabled - use 'audio lfo on' to enable"));
      }
    } else if (value == "off") {
      audio->setLFOTremoloEnabled(false);
      Serial.println(F("[OK] LFO Tremolo disabled"));
    } else {
      Serial.println(F("[ERROR] Use: on|off"));
    }
    return;

  } else if (param == "rate" || param == "freq" || param == "speed") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio lfo rate <0.1-20.0>"));
      return;
    }

    float rate = getArg(args, 1).toFloat();
    if (rate < 0.1f || rate > 20.0f) {
      Serial.println(F("[ERROR] Rate must be 0.1-20.0 Hz"));
      return;
    }

    audio->setLFORate(rate);
    Serial.printf("[OK] LFO rate: %.2f Hz\n", rate);

    if (!audio->getLFOEnabled()) {
      Serial.println(F("[HINT] LFO is disabled - use 'audio lfo on' to enable"));
    }
    return;

  } else if (param == "depth" || param == "amount" || param == "intensity") {
    if (countArgs(args) < 2) {
      Serial.println(F("[ERROR] Usage: audio lfo depth <0-100>"));
      return;
    }

    float depth = getArg(args, 1).toFloat();
    if (depth < 0.0f || depth > 100.0f) {
      Serial.println(F("[ERROR] Depth must be 0-100%"));
      return;
    }

    audio->setLFODepth(depth);
    Serial.printf("[OK] LFO depth: %.1f%%\n", depth);

    if (!audio->getLFOEnabled()) {
      Serial.println(F("[HINT] LFO is disabled - use 'audio lfo on' to enable"));
    }
    return;

  } else {
    Serial.println(F("[ERROR] Unknown parameter"));
    Serial.println(F("Use: on, off, vibrato, tremolo, rate, depth"));
  }
}
// ============================================================================
// PROFILE COMMANDS
// ============================================================================

void AudioConsole::cmdProfile(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println(F("[ERROR] Usage: audio profile <list|load|save|delete|info|export|import|validate>"));
    return;
  }

  String action = getArg(args, 0);
  action.toLowerCase();
  String name = getArg(args, 1);

  if (action == "list" || action == "ls") {
    profileManager->listProfiles();

  } else if (action == "load") {
    if (name.length() == 0) {
      Serial.println(F("[ERROR] Usage: audio profile load <name>"));
      return;
    }
    if (profileManager->loadProfile(name.c_str())) {
      Serial.println(F("[WARN] Profile loaded - restart required to apply"));
      Serial.println(F("[HINT] Type 'audio reboot' to restart"));
    }

  } else if (action == "save") {
    if (name.length() == 0) {
      Serial.println(F("[ERROR] Usage: audio profile save <name>"));
      return;
    }
    profileManager->saveProfile(name.c_str());

  } else if (action == "delete" || action == "del" || action == "rm") {
    if (name.length() == 0) {
      Serial.println(F("[ERROR] Usage: audio profile delete <name>"));
      return;
    }
    Serial.printf("[CONFIRM] Delete profile '%s'? (y/n): ", name.c_str());
    while (!Serial.available()) delay(10);
    char c = Serial.read();
    Serial.println(c);
    if (c == 'y' || c == 'Y') {
      profileManager->deleteProfile(name.c_str());
    } else {
      Serial.println(F("[CANCEL] Not deleted"));
    }

  } else if (action == "info") {
    if (name.length() == 0) {
      Serial.println(F("[ERROR] Usage: audio profile info <name>"));
      return;
    }
    profileManager->showProfileInfo(name.c_str());

  } else if (action == "export") {
    if (name.length() == 0) {
      name = String(profileManager->getCurrentSettings()->name);
    }
    profileManager->exportProfileJSON(name.c_str());

  } else if (action == "import") {
    profileManager->importProfileJSON();

  } else if (action == "validate") {
    if (name.length() == 0) {
      Serial.println(F("[ERROR] Usage: audio profile validate <name>"));
      return;
    }
    profileManager->validateProfile(name.c_str());

  } else {
    Serial.printf("[ERROR] Unknown action: %s\n", action.c_str());
  }
}

// ============================================================================
// MODE COMMANDS
// ============================================================================

void AudioConsole::cmdMode(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.printf("Current mode:   %s\n", audio->getModeName());
    return;
  }

  String mode = getArg(args, 0);
  mode.toLowerCase();

  if (mode == "info") {
    String target = getArg(args, 1);
    target.toLowerCase();

    if (target == "i2s") {
      Serial.println();
      Serial.println(F("╔════════════════════════════════════════════════════════╗"));
      Serial.println(F("║                    I2S MODE INFO                       ║"));
      Serial.println(F("╚════════════════════════════════════════════════════════╝"));
      Serial.println(F("I2S (Inter-IC Sound)"));
      Serial.println(F("  ✓ Hardware DMA-driven audio"));
      Serial.println(F("  ✓ Perfect timing (no jitter)"));
      Serial.println(F("  ✓ 100% quality"));
      Serial.println(F("  ✓ Low CPU usage (~3%)"));
      Serial.println(F("  ✓ Best for: Music, high-quality audio"));
      Serial.println();

    } else if (target == "pwm") {
      Serial.println();
      Serial.println(F("╔════════════════════════════════════════════════════════╗"));
      Serial.println(F("║                    PWM MODE INFO                       ║"));
      Serial.println(F("╚════════════════════════════════════════════════════════╝"));
      Serial.println(F("PWM (Pulse Width Modulation)"));
      Serial.println(F("  ✓ Software loop-based"));
      Serial.println(F("  ✓ 95% quality (slight jitter)"));
      Serial.println(F("  ✓ Higher CPU usage (~8%)"));
      Serial.println(F("  ✓ Best for: Sound effects, beeps"));
      Serial.println();

    } else {
      Serial.println(F("[ERROR] Usage: audio mode info <i2s|pwm>"));
    }
    return;
  }

  AudioSettings* settings = audio->getSettings();

  if (mode == "i2s") {
    settings->mode = AUDIO_MODE_I2S;
    Serial.println(F("[info] I2S activated"));
    Serial.println(F("[hint] Temporary only, for permanent use profiles & autostart"));

  } else if (mode == "pwm") {
    settings->mode = AUDIO_MODE_PWM;
    Serial.println(F("[info] PWM activated"));
    Serial.println(F("[hint] Temporary only, for permanent use profiles & autostart"));

  } else {
    Serial.printf(F("[✗] %s?\n"), mode.c_str());
  }
}

// ============================================================================
// HARDWARE, CONFIG, CODEC COMMANDS
// ============================================================================

void AudioConsole::cmdHardware(String args) {
  args.trim();
  AudioSettings* settings = audio->getSettings();

  if (args.length() == 0 || args == "show") {
    Serial.println();
    Serial.println(F("╔════════════════════════════════════════════════════════╗"));
    Serial.println(F("║              HARDWARE CONFIGURATION                    ║"));
    Serial.println(F("╚════════════════════════════════════════════════════════╝"));
    Serial.printf("Current Mode:   %s\n", settings->getModeName());

    if (settings->mode == AUDIO_MODE_I2S) {
      Serial.println(F("\nI2S Settings:"));
      Serial.printf("  Pin:          GPIO %d\n", settings->i2s.pin);
      Serial.printf("  Buffer Size:  %d samples\n", settings->i2s.bufferSize);
      Serial.printf("  Buffers:      %d\n", settings->i2s.numBuffers);
      Serial.printf("  Amplitude:    %d\n", settings->i2s.amplitude);
    } else {
      Serial.println(F("\nPWM Settings:"));
      Serial.printf("  Pin:          GPIO %d\n", settings->pwm.pin);
      Serial.printf("  Frequency:    %u Hz\n", settings->pwm.frequency);
      Serial.printf("  Resolution:   %d bits\n", settings->pwm.resolution);
      Serial.printf("  Amplitude:    %d\n", settings->pwm.amplitude);
      Serial.printf("  Gain:         %d\n", settings->pwm.gain);
    }
    Serial.println();
    return;
  }

  Serial.println(F("[INFO] Hardware configuration commands available"));
  Serial.println(F("  Use 'audio hw show' for current settings"));
}

void AudioConsole::cmdConfig(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println(F("[ERROR] Usage: audio config <startup|resample> <value>"));
    return;
  }

  String param = getArg(args, 0);
  param.toLowerCase();

  if (param == "startup") {
    String value = getArg(args, 1);
    if (value.length() == 0) {
      Serial.printf("Startup profile: %s\n", profileManager->getCurrentSettings()->name);
      return;
    }
    profileManager->setStartupProfile(value.c_str());

  } else if (param == "resample") {
    String value = getArg(args, 1);
    if (value.length() == 0) {
      Serial.println(F("[ERROR] Usage: audio config resample <quality>"));
      Serial.println(F("  Qualities: none, fast, medium, high, best"));
      return;
    }

    profileManager->getCurrentSettings()->setResampleQuality(value.c_str());
    Serial.printf("[OK] Resample quality: %s\n", value.c_str());

  } else {
    Serial.println(F("[ERROR] Unknown config parameter"));
    Serial.println(F("  Usage: audio config <param> <value>"));
  }
}

void AudioConsole::cmdCodec(String args) {
  args.trim();

  if (args.length() == 0 || args == "list") {
    codecManager->listCodecs();
    return;
  }

  String action = getArg(args, 0);
  action.toLowerCase();
  String name = getArg(args, 1);

  if (action == "info") {
    if (name.length() == 0) {
      Serial.println(F("[ERROR] Usage: audio codec info <name>"));
      return;
    }
    codecManager->showCodecInfo(name.c_str());

  } else if (action == "can") {
    if (countArgs(args) < 3) {
      Serial.println(F("[ERROR] Usage: audio codec can <dec> <file>"));
      return;
    }

    String file = getArg(args, 2);
    if (codecManager->canDecode(name.c_str(), file.c_str())) {
      Serial.printf("[OK] Codec '%s' can decode '%s'\n", name.c_str(), file.c_str());
    } else {
      Serial.printf("[ERROR] Codec '%s' cannot decode '%s'\n", name.c_str(), file.c_str());
    }

  } else {
    Serial.printf("[ERROR] Unknown action: %s\n", action.c_str());
    Serial.println(F("[HINT] Use: list, info, can"));
  }
}

// ============================================================================
// INFO & STATUS COMMANDS (UPDATED WITH LFO!)
// ============================================================================

void AudioConsole::cmdInfo(String args) {
  AudioSettings* settings = audio->getSettings();

  Serial.println();
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║              CURRENT CONFIGURATION                     ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
  Serial.println();
  Serial.printf("Profile:        %s\n", settings->name);
  Serial.printf("Audio Mode:     %s\n", settings->getModeName());
  Serial.printf("Sample Rate:    %u Hz\n", settings->sampleRate);
  Serial.printf("Voices:         %d\n", settings->voices);
  Serial.printf("Volume:         %d/255\n", settings->volume);
  Serial.printf("Waveform:       %s\n", settings->getWaveformName());

  if (settings->mode == AUDIO_MODE_I2S) {
    Serial.printf("Pin:            GPIO %d (I2S)\n", settings->i2s.pin);
    Serial.printf("Amplitude:      %d\n", settings->i2s.amplitude);
  } else {
    Serial.printf("Pin:            GPIO %d (PWM)\n", settings->pwm.pin);
    Serial.printf("Amplitude:      %d\n", settings->pwm.amplitude);
  }

  Serial.println();
  Serial.println(F("Effects:"));

  int8_t bass, mid, treble;
  audio->getEQ(bass, mid, treble);
  Serial.printf("  Biquad EQ:    %s", settings->eq.enabled ? "On" : "Off");
  if (settings->eq.enabled) {
    Serial.printf(" (B:%+d M:%+d T:%+d dB)", bass, mid, treble);
  }
  Serial.println();

  Serial.printf("  SVF Filter:   %s", settings->filter.enabled ? "On" : "Off");
  if (settings->filter.enabled) {
    Serial.printf(" (%s, %.0fHz, Q:%.2f)",
                  settings->filter.getTypeName(),
                  settings->filter.cutoff,
                  settings->filter.resonance);
  }
  Serial.println();

  Serial.printf("  Reverb:       %s", settings->reverb.enabled ? "On" : "Off");
  if (settings->reverb.enabled) {
    Serial.printf(" (Room:%.2f, Damp:%.2f, Wet:%.2f)",
                  settings->reverb.roomSize,
                  settings->reverb.damping,
                  settings->reverb.wet);
  }
  Serial.println();

  // NEW: LFO Info
  Serial.printf("  LFO:          %s", settings->lfo.enabled ? "On" : "Off");
  if (settings->lfo.enabled) {
    Serial.printf(" (");
    if (settings->lfo.vibratoEnabled) Serial.print("Vibrato");
    if (settings->lfo.vibratoEnabled && settings->lfo.tremoloEnabled) Serial.print("+");
    if (settings->lfo.tremoloEnabled) Serial.print("Tremolo");
    Serial.printf(", %.1fHz, %.0f%%)", settings->lfo.rate, settings->lfo.depth);
  }
  Serial.println();

  Serial.printf("  Delay:        %s", settings->delay.enabled ? "On" : "Off");
  if (settings->delay.enabled) {
    Serial.printf(" (%ums, FB:%u%%, Mix:%u%%)",
                  settings->delay.timeMs,
                  settings->delay.feedback,
                  settings->delay.mix);
  }
  Serial.println();

  Serial.println();
  Serial.printf("Resampling:     %s\n", settings->getResampleQualityName());
  Serial.printf("Playing:        %s\n", audio->isPlaying() ? "Yes" : "No");
  Serial.printf("Active Voices:  %d/%d\n", audio->getActiveVoices(), audio->getVoiceCount());
  Serial.println();
}

void AudioConsole::cmdStatus(String args) {
  Serial.println();
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                  SYSTEM STATUS                         ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
  Serial.println();

  Serial.printf("Uptime:         %02d:%02d:%02d\n",
                (millis() / 3600000) % 24,
                (millis() / 60000) % 60,
                (millis() / 1000) % 60);
  Serial.printf("CPU:            %s @ 160 MHz\n", ESP32_VARIANT);
  Serial.printf("Free RAM:       %d KB\n", ESP.getFreeHeap() / 1024);

  if (filesystem && filesystem->isInitialized()) {
    Serial.printf("Filesystem:     %d KB / %d KB used\n",
                  filesystem->usedBytes() / 1024,
                  filesystem->totalBytes() / 1024);
  } else {
    Serial.println(F("Filesystem:     Not mounted"));
  }

  Serial.println();
  Serial.printf("Audio Engine:   %s\n", audio->getModeName());
  Serial.printf("Sample Rate:    %u Hz\n", audio->getSampleRate());
  Serial.printf("Playing:        %s\n", audio->isPlaying() ? "Yes" : "No");
  Serial.printf("Active Voices:  %d/%d\n", audio->getActiveVoices(), audio->getVoiceCount());
  Serial.println();
}

void AudioConsole::cmdList(String args) {
  args.trim();

  if (!filesystem || !filesystem->isInitialized()) {
    Serial.println(F("[ERROR] Filesystem not mounted"));
    return;
  }

  String path = PATH_AUDIO;
  if (args.length() > 0) {
    path = args;
  }

  Serial.printf("\nListing of: %s\n", path.c_str());
  Serial.println(F("─────────────────────────────────────────────────────"));
  filesystem->listDir(path.c_str());
  Serial.println();
}

void AudioConsole::cmdTest(String args) {
  if (countArgs(args) < 2) {
    Serial.println(F("[ERROR] Usage: audio test <freq> <duration_ms>"));
    return;
  }

  int freq = getArg(args, 0).toInt();
  int duration = getArg(args, 1).toInt();

  if (freq < 20 || freq > 20000) {
    Serial.println(F("[ERROR] Frequency must be 20-20000 Hz"));
    return;
  }

  Serial.printf("[TEST] Playing %d Hz tone for %d ms\n", freq, duration);

  float midi = 69.0f + 12.0f * log2f((float)freq / 440.0f);
  uint8_t note = (uint8_t)roundf(midi);

  audio->noteOn(note, 127);
  delay(duration);
  audio->noteOff(note);

  Serial.println(F("[TEST] Done"));
}

void AudioConsole::cmdVersion(String args) {
  Serial.println();
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                  ESP32 AUDIO OS                        ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
  Serial.println();
  Serial.printf("Version:        %s\n", AUDIO_OS_VERSION);
  Serial.printf("Build Date:     %s\n", AUDIO_OS_BUILD_DATE);
  Serial.printf("Schema:         %s\n", PROFILE_SCHEMA_VERSION);
  Serial.printf("Target:         %s\n", ESP32_VARIANT);
  Serial.printf("Cores:          %s\n", ESP32_HAS_DUAL_CORE ? "Dual" : "Single");
  Serial.println();
  Serial.println(F("Features:"));
  Serial.println(F("  ✓ Profile system"));
  Serial.println(F("  ✓ I2S & PWM support"));
  Serial.println(F("  ✓ 5 Waveforms (Sine/Square/Saw/Tri/Noise)"));
  Serial.println(F("  ✓ State-Variable Filter (LP/HP/BP)"));
  Serial.println(F("  ✓ Biquad EQ (3-band parametric)"));
  Serial.println(F("  ✓ Schroeder Reverb (Comb+Allpass)"));
  Serial.println(F("  ✓ LFO Modulation (Vibrato/Tremolo)"));  // NEW!
  Serial.println(F("  ✓ Delay/Echo effect"));
  Serial.println(F("  ✓ Smart resampling"));
  Serial.println(F("  ✓ Codec plugins"));
  Serial.println(F("  ✓ Full console control"));
  Serial.println(F("  ✓ Fixed-point math optimization"));
  Serial.println(F("  ✓ Wavetable synthesis"));
  Serial.println();
  Serial.println(F("Built-in Codecs:"));
  Serial.println(F("  ✓ WAV (PCM)"));
  Serial.println();
  Serial.println(F("License:        MIT"));
  Serial.println(F("Author:         AI-Generated (Perplexity)"));
  Serial.println();
}

void AudioConsole::cmdReset(String args) {
  Serial.print(F("[CONFIRM] Reset to factory defaults? (y/n): "));
  while (!Serial.available()) delay(10);
  char c = Serial.read();
  Serial.println(c);

  if (c == 'y' || c == 'Y') {
    Serial.println(F("[RESET] Creating default profile..."));
    profileManager->createDefaultProfile();
    Serial.println(F("[RESET] Done - type 'audio reboot' to restart"));
  } else {
    Serial.println(F("[CANCEL] Not reset"));
  }
}

void AudioConsole::cmdReboot(String args) {
  Serial.print(F("[CONFIRM] Reboot ESP32? (y/n): "));
  while (!Serial.available()) delay(10);
  char c = Serial.read();
  Serial.println(c);

  if (c == 'y' || c == 'Y') {
    Serial.println(F("[REBOOT] Restarting in 2 seconds..."));
    delay(2000);
    ESP.restart();
  } else {
    Serial.println(F("[CANCEL] Not rebooted"));
  }
}
// ============================================================================
// HELP COMMAND (UPDATED WITH LFO!)
// ============================================================================

void AudioConsole::cmdHelp(String args) {
  args.trim();

  if (args.length() == 0) {
    Serial.println();
    Serial.println(F("╔════════════════════════════════════════════════════════╗"));
    Serial.println(F("║           ESP32 AUDIO OS - COMMAND REFERENCE           ║"));
    Serial.println(F("╚════════════════════════════════════════════════════════╝"));
    Serial.println();
    Serial.println(F("PLAYBACK:"));
    Serial.println(F("  audio play [file]        Play file or built-in melody"));
    Serial.println(F("  audio stop               Stop playback"));
    Serial.println(F("  audio volume <0-255>     Set volume"));
    Serial.println(F("  audio note <0-127> [ms]  Play MIDI note"));
    Serial.println(F("  audio waveform <type>    Set waveform"));
    Serial.println();
    Serial.println(F("EFFECTS:"));
    Serial.println(F("  audio filter trol>   State-Variable Filter (LP/HP/BP)"));
    Serial.println(F("  audio eq <on|off|band>   Biquad EQ control"));
    Serial.println(F("  audio reverb trol>   Schroeder Reverb (Hall)"));
    Serial.println(F("  audio lfo trol>      LFO Vibrato/Tremolo (NEW!)"));
    Serial.println(F("  audio delay <on|off>     Delay/Echo effect"));
    Serial.println();
    Serial.println(F("PROFILES:"));
    Serial.println(F("  audio profile list       List all profiles"));
    Serial.println(F("  audio profile load <name> Load profile"));
    Serial.println(F("  audio profile save <name> Save current settings"));
    Serial.println(F("  audio profile info <name> Show profile details"));
    Serial.println();
    Serial.println(F("CONFIGURATION:"));
    Serial.println(F("  audio mode <i2s|pwm>     Switch audio mode"));
    Serial.println(F("  audio hw show            Show hardware settings"));
    Serial.println(F("  audio config resample <q> Set resample quality"));
    Serial.println();
    Serial.println(F("CODECS:"));
    Serial.println(F("  audio codec list         List available codecs"));
    Serial.println(F("  audio codec info <name>  Show codec details"));
    Serial.println();
    Serial.println(F("SYSTEM:"));
    Serial.println(F("  audio info               Current configuration"));
    Serial.println(F("  audio status             System status"));
    Serial.println(F("  audio list [path]        List audio files"));
    Serial.println(F("  audio version            Show version"));
    Serial.println(F("  audio reset              Factory reset"));
    Serial.println(F("  audio reboot             Restart ESP32"));
    Serial.println();
    Serial.println(F("EXAMPLES:"));
    Serial.println(F("  audio play               Play Tetris (default)"));
    Serial.println(F("  audio waveform square    8-bit retro sound"));
    Serial.println(F("  audio filter lowpass     Enable lowpass filter"));
    Serial.println(F("  audio reverb on          Enable hall reverb"));
    Serial.println(F("  audio reverb room 0.8    Large hall"));
    Serial.println(F("  audio lfo vibrato on     Enable pitch wobble"));
    Serial.println(F("  audio lfo rate 6.0       6 Hz modulation"));
    Serial.println(F("  audio eq bass +6         Bass boost +6dB"));
    Serial.println();

  } else {
    String cmd = args;
    cmd.toLowerCase();

    if (cmd == "reverb") {
      Serial.println();
      Serial.println(F("audio reverb [on|off|room|damping|wet]"));
      Serial.println(F("Schroeder reverb (Comb + Allpass filters)."));
      Serial.println();
      Serial.println(F("PARAMETERS:"));
      Serial.println(F("  room <n>     - Room size 0.0-1.0 (feedback)"));
      Serial.println(F("  damping <n>  - High-freq damping 0.0-1.0"));
      Serial.println(F("  wet <n>      - Wet/Dry mix 0.0-1.0"));
      Serial.println();
      Serial.println(F("EXAMPLES:"));
      Serial.println(F("  audio reverb on"));
      Serial.println(F("  audio reverb room 0.7    # Cathedral"));
      Serial.println(F("  audio reverb damping 0.5 # Medium damping"));
      Serial.println(F("  audio reverb wet 0.4     # 40% reverb"));
      Serial.println();

    } else if (cmd == "lfo") {
      Serial.println();
      Serial.println(F("audio lfo [on|off|vibrato|tremolo|rate|depth]"));
      Serial.println(F("Low-Frequency Oscillator for modulation effects."));
      Serial.println();
      Serial.println(F("PARAMETERS:"));
      Serial.println(F("  vibrato <on|off> - Pitch modulation (wobble)"));
      Serial.println(F("  tremolo <on|off> - Amplitude modulation (pulse)"));
      Serial.println(F("  rate <n>         - LFO speed 0.1-20.0 Hz"));
      Serial.println(F("  depth <n>        - Intensity 0-100%"));
      Serial.println();
      Serial.println(F("EXAMPLES:"));
      Serial.println(F("  audio lfo on"));
      Serial.println(F("  audio lfo vibrato on     # Enable pitch wobble"));
      Serial.println(F("  audio lfo tremolo on     # Enable volume pulse"));
      Serial.println(F("  audio lfo rate 5.0       # 5 Hz modulation"));
      Serial.println(F("  audio lfo depth 30       # 30% intensity"));
      Serial.println();
      Serial.println(F("TIP: Vibrato = Singing voice effect"));
      Serial.println(F("     Tremolo = Guitar amp effect"));
      Serial.println();

    } else if (cmd == "filter") {
      Serial.println();
      Serial.println(F("audio filter [on|off|type|cutoff|resonance]"));
      Serial.println(F("State-Variable Filter (LP/HP/BP)."));
      Serial.println();
      Serial.println(F("PARAMETERS:"));
      Serial.println(F("  lowpass      - Low-pass filter"));
      Serial.println(F("  highpass     - High-pass filter"));
      Serial.println(F("  bandpass     - Band-pass filter"));
      Serial.println(F("  cutoff <n>   - Cutoff frequency 20-20000 Hz"));
      Serial.println(F("  resonance <n>- Resonance 0.0-1.0"));
      Serial.println();
      Serial.println(F("EXAMPLES:"));
      Serial.println(F("  audio filter on"));
      Serial.println(F("  audio filter lowpass"));
      Serial.println(F("  audio filter cutoff 800"));
      Serial.println(F("  audio filter resonance 0.7"));
      Serial.println();

    } else if (cmd == "eq") {
      Serial.println();
      Serial.println(F("audio eq [on|off|bass|mid|treble]"));
      Serial.println(F("3-band Biquad EQ."));
      Serial.println();
      Serial.println(F("PARAMETERS:"));
      Serial.println(F("  bass <n>     - Bass gain -12 to +12 dB (120 Hz)"));
      Serial.println(F("  mid <n>      - Mid gain -12 to +12 dB (1000 Hz)"));
      Serial.println(F("  treble <n>   - Treble gain -12 to +12 dB (8000 Hz)"));
      Serial.println();
      Serial.println(F("EXAMPLES:"));
      Serial.println(F("  audio eq on"));
      Serial.println(F("  audio eq bass +6"));
      Serial.println(F("  audio eq mid -3"));
      Serial.println(F("  audio eq treble +4"));
      Serial.println();

    } else if (cmd == "waveform" || cmd == "wave") {
      Serial.println();
      Serial.println(F("audio waveform <sine|square|sawtooth|triangle|noise>"));
      Serial.println(F("Set oscillator waveform."));
      Serial.println();
      Serial.println(F("WAVEFORMS:"));
      Serial.println(F("  sine      - Pure sine wave (smooth)"));
      Serial.println(F("  square    - Square wave (8-bit retro)"));
      Serial.println(F("  sawtooth  - Sawtooth wave (bright)"));
      Serial.println(F("  triangle  - Triangle wave (mellow)"));
      Serial.println(F("  noise     - White noise"));
      Serial.println();
      Serial.println(F("EXAMPLES:"));
      Serial.println(F("  audio waveform sine"));
      Serial.println(F("  audio waveform square"));
      Serial.println();

    } else {
      Serial.printf("No detailed help for: %s\n", cmd.c_str());
      Serial.println(F("Use 'audio help' for overview"));
    }
    Serial.println();
  }
}


// ============================================================================
// MELODY JSON LOADER
// ============================================================================
bool AudioConsole::loadAndPlayMelody(const char* path) {
  if (!filesystem || !filesystem->isInitialized()) {
    Serial.println(F("ERROR: Filesystem not mounted"));
    return false;
  }

  // Open file with mode parameter (required by AudioFilesystem)
  File file = filesystem->open(path, "r");
  if (!file) {
    Serial.printf("ERROR: Cannot open file: %s\n", path);
    return false;
  }

  // Parse JSON
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.printf("ERROR: JSON parse failed: %s\n", error.c_str());
    return false;
  }

  // Validate melody structure
  if (!doc.containsKey("notes")) {
    Serial.println(F("ERROR: JSON missing 'notes' array"));
    return false;
  }

  JsonArray notesArray = doc["notes"];
  size_t noteCount = notesArray.size();

  if (noteCount == 0) {
    Serial.println(F("ERROR: Empty notes array"));
    return false;
  }

  // Allocate temporary melody buffer
  Note* melody = new Note[noteCount];
  if (!melody) {
    Serial.println(F("ERROR: Out of memory for melody"));
    return false;
  }

  // Parse notes
  for (size_t i = 0; i < noteCount; i++) {
    JsonObject note = notesArray[i];
    melody[i].pitch = note["freq"] | NOTE_REST;
    melody[i].duration = note["duration"] | 500;
    melody[i].velocity = note["velocity"] | 127;
  }

  // Show melody info
  String name = doc["name"] | "Unknown";
  Serial.printf("AUDIO: Playing '%s' (%d notes)\n", name.c_str(), noteCount);

  // Play melody (MelodyPlayer will copy and own the data)
  audio->playMelody(melody, noteCount);

  // Clean up temporary buffer (MelodyPlayer has its own copy now)
  delete[] melody;

  return true;
}

// ============================================================================
// SCHEDULED NOTE MANAGEMENT
// ============================================================================

void AudioConsole::scheduleNoteOff(uint8_t note, uint32_t durationMs) {
  for (int i = 0; i < MAX_SCHEDULED_NOTES; i++) {
    if (!scheduledNotes[i].active) {
      scheduledNotes[i].note = note;
      scheduledNotes[i].stopTime = millis() + durationMs;
      scheduledNotes[i].active = true;
      return;
    }
  }

  // If all slots full, overwrite first slot
  scheduledNotes[0].note = note;
  scheduledNotes[0].stopTime = millis() + durationMs;
  scheduledNotes[0].active = true;
}

void AudioConsole::updateScheduledNotes() {
  uint32_t now = millis();

  for (int i = 0; i < MAX_SCHEDULED_NOTES; i++) {
    if (scheduledNotes[i].active) {
      if (now >= scheduledNotes[i].stopTime) {
        audio->noteOff(scheduledNotes[i].note);
        scheduledNotes[i].active = false;
      }
    }
  }
}
