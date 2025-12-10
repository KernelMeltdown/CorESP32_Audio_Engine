#ifndef AUDIOCODEC_MP3_H
#define AUDIOCODEC_MP3_H

#include "AudioCodec.h"
#include "minimp3.h"

class AudioCodecMP3 : public AudioCodec {
private:
  mp3dec_t mp3d;
  mp3dec_frame_info_t info;
  File file;
  uint8_t buffer[MINIMP3_IO_SIZE*2];
  size_t bytes_left;
  
public:
  AudioCodecMP3() : bytes_left(0) { mp3d = mp3dec_init(NULL); }
  
  bool canDecode(const char* filename) override {
    return strstr(filename, ".mp3") != NULL;
  }
  
  bool open(const char* filename) override {
    file = filesystem->open(filename, "r");
    if (!file) return false;
    bytes_left = file.size();
    return true;
  }
  
  int16_t readSample() override {
    if (info.frame_bytes == 0) {
      // Decode next frame
      size_t bytes_read = file.read(buffer, sizeof(buffer));
      if (bytes_read == 0) return 0; // EOF
      
      int samples = mp3dec_decode_frame(&mp3d, buffer, bytes_read, NULL, &info);
      if (samples <= 0) return 0;
    }
    
    // Mono downmix + return sample
    int16_t left = (int16_t)info.buffer[0];
    int16_t right = (int16_t)info.buffer[1];
    info.frame_bytes--;
    return (left + right) / 2;
  }
  
  AudioFormat getFormat() override {
    return {info.hz, 16, 1, "MP3"};
  }
};

#endif
