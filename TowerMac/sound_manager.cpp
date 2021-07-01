//
//  sound_manager.cpp
//  TowerMac
//
//  Created by Zeal Iskander on 01/07/2021.
//

#include "sound_manager.hpp"
#include <iostream>

const size_t PRIORITY_CAP = 10;

struct sample
{
    Uint8 *data;
    Uint32 dlen = 0;
    bool repeat = false;
    Uint32 dpos = 0;
} sound[PRIORITY_CAP];

const sound_manager sound_manager::sm;

bool sound_manager::currently_playing(size_t priority){return sound[priority].dpos != sound[priority].dlen;};

void sound_manager::mix_audio(void *sample1, Uint8 *stream, int len)
{
    Uint32 amount;
    auto priority = PRIORITY_CAP;
    while (priority!=0)
    {
        priority--;
        if (!currently_playing(priority))
        {
            if (sound[priority].repeat) // we reset the first repeating sound
                sound[priority].dpos = 0;
            else
                continue;
        }
        amount = (sound[priority].dlen-sound[priority].dpos);
        if (amount > len)
            amount = len;
        memcpy(stream, sound[priority].data+sound[priority].dpos, amount);
        sound[priority].dpos += amount;
        return;
    }
    for (auto i = 0; i!=len; i++) // no viable priority means we must play silence instead.
        stream[i] = 128;
    return;
}

void sound_manager::play_sound(char *file, size_t priority, bool repeat) const
{
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;

    if (sound[priority].dpos != sound[priority].dlen)
        return;

    if (SDL_LoadWAV(file, &wave, &data, &dlen) == NULL)
    {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        exit(1);
    }
    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq, spec_.format, spec_.channels, spec_.freq);
    cvt.len = dlen;
    cvt.buf = (unsigned char *)malloc(cvt.len*cvt.len_mult);
    memcpy(cvt.buf, data, cvt.len);
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(data);

    if (sound[priority].data)
        free(sound[priority].data);
    sound[priority] = {cvt.buf, (Uint32)cvt.len_cvt, repeat};
}
