// ============================================================================
// AUDIO CONSOLE - Header v1.9
// ============================================================================
#ifndef AUDIOCONSOLE_H
#define AUDIOCONSOLE_H

#include <Arduino.h>

// Forward declarations
class AudioEngine;
class AudioProfile;
class AudioFilesystem;
class AudioCodecManager;

// Scheduled note for delayed note-off
struct ScheduledNote {
  uint8_t note;
  uint32_t stopTime;
  bool active;
};

#define MAX_SCHEDULED_NOTES 8

class AudioConsole {
private:
  AudioEngine* audio;
  AudioProfile* profileManager;
  AudioFilesystem* filesystem;
  AudioCodecManager* codecManager;

  String cmdBuffer;
  ScheduledNote scheduledNotes[MAX_SCHEDULED_NOTES];

  // Command parsing
  String getArg(const String& input, int index);
  int countArgs(const String& input);
  void processCommand(String cmd);

  // Command handlers
  void cmdHelp(String args);
  void cmdInfo(String args);
  void cmdStatus(String args);
  void cmdVersion(String args);
  void cmdPlay(String args);
  void cmdStop(String args);
  void cmdVolume(String args);
  void cmdNote(String args);
  void cmdWaveform(String args);
  void cmdEQ(String args);
  void cmdFilter(String args);
  void cmdReverb(String args);
  void cmdLFO(String args);
  void cmdDelay(String args);
  void cmdProfile(String args);
  void cmdMode(String args);
  void cmdHardware(String args);
  void cmdConfig(String args);
  void cmdCodec(String args);
  void cmdList(String args);
  void cmdTest(String args);
  void cmdReset(String args);
  void cmdReboot(String args);

  // Melody loading
  bool loadAndPlayMelody(const char* path);

  // Scheduled notes
  void scheduleNoteOff(uint8_t note, uint32_t durationMs);
  void updateScheduledNotes();

public:
  AudioConsole();
  void init(AudioEngine* audioEngine, AudioProfile* profMgr, AudioFilesystem* fs, AudioCodecManager* codecMgr);
  void update();
};

#endif // AUDIOCONSOLE_H
