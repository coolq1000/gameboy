#include "audio.hpp"
#include <cmath>

audio::audio(gmb::apu& _apu) : apu(_apu)
{
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = _apu.sample_rate;
    spec.format = AUDIO_FORMAT;
    spec.channels = AUDIO_CHANNELS;
    spec.samples = _apu.latency;
    spec.userdata = &_apu;
    spec.callback = write;

    device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    SDL_PauseAudioDevice(device, 0);
}

audio::~audio()
{
    SDL_CloseAudioDevice(device);
}

void audio::write(void *userdata, Uint8* stream, int len)
{
    auto* apu = reinterpret_cast<gmb::apu*>(userdata);

    for (usize i = 0; i < len / sizeof(i16); i++)
    {
        i16 sample = (apu->core_apu.flip ? apu->core_apu.buffer1 : apu->core_apu.buffer2)[i];
        *(reinterpret_cast<i16*>(stream) + i) = sample;
    }
}

void audio::queue(i16 sample)
{
    SDL_QueueAudio(device, &sample, sizeof(i16));
}

u32 audio::queued()
{
    return SDL_GetQueuedAudioSize(device);
}
