//
//  sound_manager.cpp
//  TowerMac
//
//  Created by Zeal Iskander on 01/07/2021.
//

#include "sound_manager.hpp"
#include <iostream>

struct sample
{
    Uint8 *data;
    Uint32 dlen;
    Uint32 dpos = 0;
} sound;

bool sound_manager::currently_playing(){return sound.dpos != sound.dlen;};

void sound_manager::mixaudio(void *sample1, Uint8 *stream, int len)
{
    if ( !currently_playing() ) // if no sound is queued we need to manually set the stream to a silence.
    {
        auto silence = calloc(len, sizeof(Uint8));
        memcpy(stream, silence, len);
        return;
    }
    Uint32 amount;
    amount = (sound.dlen-sound.dpos);
    if ( amount > len )
        amount = len;
    memcpy(stream, sound.data, amount);
    sound.dpos += amount;
}

void sound_manager::playsound(char *file)
{
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;

    if ( sound.dpos != sound.dlen ) {
        return;
    }

    if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        exit(1);
    }
    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq, spec_.format, spec_.channels, spec_.freq);
    cvt.buf = (unsigned char *)malloc(dlen*cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(data);

    if ( sound.data ) {
        free(sound.data);
    }
    sound = {cvt.buf, (Uint32)cvt.len_cvt};
}
