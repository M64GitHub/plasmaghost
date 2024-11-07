// resid-audio.cpp, 2023 M64

#include "sdl-audio.h"
#include "audio-config.h"
#include "resid.h"

void audio_callback(void *userdata, uint8_t *stream, int len) {
  ReSIDPbData *D = (ReSIDPbData *)userdata;

  D->stat_cnt++;

  if (!D->play)
    return;

  if (D->buf_lock) {
    D->stat_buf_underruns++;
    return;
  }

  // play audio buffer
  memcpy(stream, (uint8_t *)D->buf_next, len);

  // switch buffers
  if (D->buf_next == D->buf1) {
    D->buf_next = D->buf2;
    D->buf_playing = D->buf1;
  } else {
    D->buf_next = D->buf1;
    D->buf_playing = D->buf2;
  }

  D->stat_bufwrites++;
  D->buf_consumed = 1;
}

int sdl_audio_init(SDL_AudioDeviceID *id, SDL_AudioSpec *spec_out,
                   ReSIDPbData *D) {
  if (SDL_Init(SDL_INIT_AUDIO)) {
    printf("ERROR initializing SDL_AUDIO subsystem: ");
    printf("%s\n", SDL_GetError());
    return 2;
  }

  printf("[AUDIO] SDL2 audio subsystem initialized\n");

  // configure audio device struct
  SDL_AudioSpec spec_in;
  memset(spec_out, 0, sizeof(SDL_AudioSpec));

  spec_in.freq = CFG_AUDIO_SAMPLING_RATE;
  spec_in.format = AUDIO_S16;           // signed 16bit little endian
  spec_in.channels = 1;                 // mono / stereo
  spec_in.samples = CFG_AUDIO_BUF_SIZE; // size in samples
  spec_in.userdata = D;
  spec_in.callback = audio_callback;

  *id = SDL_OpenAudioDevice(NULL, 0, &spec_in, spec_out, 0);

  if (*id < 1) {
    printf("[ERR] initializing audio device: %s", SDL_GetError());
    return 1;
  }
  printf("[AUDIO] SDL2 audio device opened: id: ");
  printf("%d\n", *id);

  printf("[AUDIO] audio spec: ");
  printf("bufsize_s: %d, bufsize_b: %d, freq: %d\n", spec_out->samples,
         spec_out->size, spec_out->freq);

  // audio devices default to being paused, so turn off pause
  SDL_PauseAudioDevice(*id, 0);

  return 0;
}
