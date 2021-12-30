#include "audio.hpp"

bool audio_stream::onGetData(Chunk& data)
{
    /* emit samples into buffer */
    for (usize seek = 0; seek < sample_rate; seek++)
    {
        samples[seek] = apu.emit();
    }

    data.samples = &samples[current_sample];
    data.sampleCount = sample_rate;
    return true;
}

void audio_stream::onSeek(sf::Time timeOffset)
{
    current_sample = static_cast<usize>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
}

audio_stream::audio_stream(gmb::apu apu) : apu(apu)
{
    load();
}

void audio_stream::load()
{
    /* fill with empty samples */
    samples.resize(sample_rate);

    /* reset current sample to beginning */
    current_sample = 0;

    /* call back to the super (sf::SoundStream) for initialization */
    initialize(1, sample_rate);
}
