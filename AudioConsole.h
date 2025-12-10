/*
 ╔══════════════════════════════════════════════════════════════════════════════╗
 ║  AUDIO CONSOLE - Header v1.9                                                 ║
 ║  Serial Console Interface with LFO Support                                   ║
 ╚══════════════════════════════════════════════════════════════════════════════╝
*/

#ifndef AUDIO_CONSOLE_H
#define AUDIO_CONSOLE_H

#include <Arduino.h>
#include "AudioConfig.h"

// Forward declarations
class AudioEngine;
class AudioProfile;
class AudioFilesystem;
class AudioCodecManager;

// Console configuration
#define CONSOLE_PROMPT "audio> "
#define CONSOLE_MAX_CMD_LEN 128

// Scheduled note structure
struct ScheduledNote {
  uint8_t note;
  uint32_t stopTime;
  bool active;
};

#define MAX_SCHEDULED_NOTES 16

class AudioConsole {
public:
  AudioConsole();
  
  // ✅ FIXED: Added AudioCodecManager parameter
  void init(AudioEngine* engine, AudioProfile* profile, AudioFilesystem* fs, AudioCodecManager* codec);
  void update();

private:
  // Core references
  AudioEngine* audio;
  AudioProfile* profileManager;
  AudioFilesystem* filesystem;
  AudioCodecManager* codecManager;  // ✅ ADDED
  
  // Console state
  String cmdBuffer;
  ScheduledNote scheduledNotes[MAX_SCHEDULED_NOTES];
  
  // Command processing
  void processCommand(String cmd);
  
  // Playback commands
  void cmdPlay(String args);
  void cmdStop(String args);
  void cmdVolume(String args);
  void cmdNote(String args);
  void cmdWaveform(String args);
  
  // Effect commands
  void cmdFilter(String args);
  void cmdEQ(String args);
  void cmdReverb(String args);
  void cmdLFO(String args);      // ✅ ADDED
  void cmdDelay(String args);
  
  // System commands
  void cmdProfile(String args);
  void cmdMode(String args);
  void cmdHardware(String args);
  void cmdConfig(String args);
  void cmdCodec(String args);
  void cmdInfo(String args);
  void cmdStatus(String args);
  void cmdList(String args);
  void cmdTest(String args);
  void cmdVersion(String args);
  void cmdHelp(String args);
  void cmdReset(String args);
  void cmdReboot(String args);
  
  // ✅ FIXED: const String& parameter
  String getArg(const String& input, int index);
  int countArgs(const String& input);
  
  // Note scheduling
  void scheduleNoteOff(uint8_t note, uint32_t durationMs);
  void updateScheduledNotes();
};

#endif // AUDIO_CONSOLE_H
