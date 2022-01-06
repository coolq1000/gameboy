#include "audio.hpp"

bool audio_stream::onGetData(Chunk& data)
{
    data.samples = &apu->core_apu.buffer[current_sample];
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
    /* reset current sample to beginning */
    current_sample = 0;

    /* call back to the super (sf::SoundStream) for initialization */
    initialize(1, apu->sample_rate);
}
