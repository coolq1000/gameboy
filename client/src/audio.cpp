#include "audio.hpp"
#include <cmath>

audio::audio(gmb::apu& _apu) : apu(_apu)
{
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = AUDIO_FORMAT;
    config.playback.channels = AUDIO_CHANNELS;
    config.sampleRate = _apu.sample_rate;
    config.dataCallback = write;
    config.pUserData = this;

    ma_device_init(NULL, &config, &device);
    ma_device_start(&device);
}

audio::~audio()
{
    ma_device_uninit(&device);
}

void audio::write(ma_device* device, void* output, const void* input, ma_uint32 frame_count)
{
    auto* _audio = reinterpret_cast<audio*>(device->pUserData);

    for (usize i = 0; i < frame_count; i++)
    {
        *((i16*)(output) + i) = ((i16*)_audio->apu.core_apu.buffer)[i];
    }
}
