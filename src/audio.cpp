#include "audio.hpp"
#include <cmath>
#include <vector>

Audio::Audio(gmb::APU& apu) : apu(apu)
{
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = apu.sample_rate;
    spec.format = AUDIO_FORMAT;
    spec.channels = AUDIO_CHANNELS;
    spec.samples = apu.latency;
    spec.userdata = &apu;
    spec.callback = nullptr;

    device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    SDL_PauseAudioDevice(device, 0);
}

Audio::~Audio()
{
    SDL_CloseAudioDevice(device);
}

void Audio::queue(i16* sample, usize length)
{
    SDL_QueueAudio(device, sample, length * sizeof(i16));
}

usize Audio::queued()
{
    return SDL_GetQueuedAudioSize(device) * sizeof(i16);
}
