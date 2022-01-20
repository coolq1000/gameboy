
#ifndef AUDIO_HPP
#define AUDIO_HPP

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "core/util.h"
#include "core/dmg.hpp"

#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 2

class audio
{
    SDL_AudioDeviceID device;

public:

    gmb::apu apu;

    audio(gmb::apu& _apu);
    ~audio();

    void queue(i16* sample, usize length);
    usize queued();
};

#endif
