
#ifndef APU_H
#define APU_H

#include "util.h"

typedef struct channel
{
	i16 (*sample_callback)(u32 seek, usize sample_rate);
	bool enabled;
} channel_t;

typedef struct apu
{
	channel_t ch1; /* tone & sweep */
	channel_t ch2; /* tone */
	channel_t ch3; /* wave output */
	channel_t ch4; /* noise */
} apu_t;

void apu_create(apu_t* apu);
void apu_destroy(apu_t* apu);

i16 sine_wave(u32 seek, usize sample_rate);
i16 sawtooth_wave(u32 seek, usize sample_rate);
i16 triangle_wave(u32 seek, usize sample_rate);
i16 square_wave(u32 seek, usize sample_rate);

#endif
