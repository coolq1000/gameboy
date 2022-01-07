
#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SDL.h>
#include "core/util.h"
#include "core/dmg.hpp"

#define AUDIO_FORMAT AUDIO_S16SYS
#define AUDIO_CHANNELS 1

class audio
{

    SDL_AudioDeviceID device;

public:

    gmb::apu apu;

    audio(gmb::apu& _apu);
    ~audio();

    static void write(void *userdata, Uint8* stream, int len);

    void queue(i16 sample);
    u32 queued();
};

#endif
