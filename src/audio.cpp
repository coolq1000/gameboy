#include "audio.hpp"
#include <cmath>
#include <vector>

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
    spec.callback = nullptr;

    device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    SDL_PauseAudioDevice(device, 0);
}

audio::~audio()
{
    SDL_CloseAudioDevice(device);
}

void audio::queue(i16* sample, usize length)
{
    SDL_QueueAudio(device, sample, length * sizeof(i16));
}

usize audio::queued()
{
    return SDL_GetQueuedAudioSize(device) * sizeof(i16);
}
