
#ifndef AUDIO_HPP
#define AUDIO_HPP

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "core/util.h"
#include "core/dmg.hpp"

#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 2

struct Audio
{
    SDL_AudioDeviceID device;

    gmb::APU apu;

    Audio(gmb::APU& apu);
    ~Audio();

    void queue(i16* sample, usize length);
    usize queued();
};

#endif
