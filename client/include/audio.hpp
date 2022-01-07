
#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SFML/Audio.hpp>
#include "core/util.h"
#include "core/dmg.hpp"
#define MA_NO_DECODING
#define MA_NO_ENCODING
#include <miniaudio/miniaudio.h>

#define AUDIO_FORMAT ma_format_s16
#define AUDIO_CHANNELS 1

class audio
{
    ma_device device;

public:

    audio(gmb::apu& _apu);
    ~audio();

    static void write(ma_device* device, void* output, const void* input, ma_uint32 frame_count);
};

#endif
