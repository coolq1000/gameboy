#include "audio.hpp"

bool audio_stream::onGetData(Chunk& data)
{
    /* copy samples into buffer */
    for (usize seek = 0; seek < apu->latency; seek++)
    {
        samples[seek] = apu->core_apu.buffer[seek];
    }

    data.samples = &samples[current_sample];
    data.sampleCount = apu->latency;
    return true;
}

void audio_stream::onSeek(sf::Time timeOffset)
{
    current_sample = static_cast<usize>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
}

audio_stream::audio_stream(gmb::apu* apu)
{
    this->apu = apu;
    load();
}

void audio_stream::load()
{
    /* fill with empty samples */
    samples.resize(apu->latency);

    /* reset current sample to beginning */
    current_sample = 0;

    /* call back to the super (sf::SoundStream) for initialization */
    initialize(1, apu->sample_rate);
}
