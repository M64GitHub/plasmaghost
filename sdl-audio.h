// resid-audio.h, 2023 M64

#ifndef SDL_AUDIO_H
#define SDL_AUDIO_H

#ifdef __APPLE__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include "resid.h"

int sdl_audio_init(SDL_AudioDeviceID *id,
                   SDL_AudioSpec     *spec,
                   ReSIDPbData       *R);

#endif

