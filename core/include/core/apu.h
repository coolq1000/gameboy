
#ifndef APU_H
#define APU_H

#include "util.h"

typedef struct
{
	i16 (*sample_callback)(u8 seek);
	bool enabled;
} channel_t;

typedef struct
{
	channel_t ch1; /* tone & sweep */
	channel_t ch2; /* tone */
	channel_t ch3; /* wave output */
	channel_t ch4; /* noise */
} apu_t;

void apu_create(apu_t* apu);
void apu_destroy(apu_t* apu);

i16 sine_wave(u8 seek);

#endif
