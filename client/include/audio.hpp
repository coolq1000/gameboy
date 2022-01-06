
#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SFML/Audio.hpp>
#include "core/util.h"
#include "core/dmg.hpp"
#include <al/al.h>

class audio_stream : public sf::SoundStream
{
    usize current_sample;

    gmb::apu* apu;

    virtual bool onGetData(Chunk& data);
    virtual void onSeek(sf::Time timeOffset);

public:

    audio_stream(gmb::apu* apu);

    void load();
};

#endif
