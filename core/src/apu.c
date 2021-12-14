#include "core/apu.h"

#include <math.h>

void apu_create(apu_t* apu)
{
    apu->ch1.sample_callback = sine_wave;
}

void apu_destroy(apu_t* apu)
{
	
}

i16 sine_wave(u8 seek)
{
    float pitch = seek * 10.0f;
    float volume = 0.1f;

    float tpc = 44100.0f / pitch;
    float cycles = ((float)seek) / tpc;
    float rad = 6.2831f * cycles;

    i16 amp = 32767.0f * volume;

    return amp * sin(rad);
}
