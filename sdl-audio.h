// resid-audio.h, 2023 M64

#ifndef SDL_AUDIO_H
#define SDL_AUDIO_H

#include <SDL2/SDL.h>
#include "resid.h"

int sdl_audio_init(SDL_AudioDeviceID *id, 
                   SDL_AudioSpec     *spec,
                   ReSIDPbData       *R);

#endif

