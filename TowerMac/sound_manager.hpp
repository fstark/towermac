//
//  sound_manager.hpp
//  TowerMac
//
//  Created by Zeal Iskander on 01/07/2021.
//

#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"

#ifndef SOUND_MANAGER_INCLUDED__
#define SOUND_MANAGER_INCLUDED__

#include <stdio.h>

class sound_manager
{
    SDL_AudioSpec spec_;
    static void mix_audio(void *sample1, Uint8 *stream, int len);
    static bool currently_playing(size_t priority);
    sound_manager()
    {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
            throw "Could not initialize audio";
        spec_.freq = 22200;
        spec_.format = AUDIO_U8;
        spec_.channels = 1;
        spec_.samples = 370;
        spec_.callback = mix_audio;
        if ( SDL_OpenAudio(&spec_, &spec_) < 0 ) {
            fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
            exit(1);
        }
        SDL_PauseAudio(0);
    }
public:
    static const sound_manager sm;
    void play_sound(char *file, size_t priority, bool repeat = false) const;
};

#endif
