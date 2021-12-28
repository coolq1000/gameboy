
#ifndef APU_H
#define APU_H

#include "util.h"

typedef struct apu apu_t;

typedef struct timer
{
    u16 period, counter;
} timer_t;

typedef struct duty
{
    timer_t timer;
    u8 counter, cycle;
    u16 frequency;
    bool enabled;
} duty_t;

typedef struct sweep
{
    timer_t timer;
    u16 period, frequency, shift;
    bool decreasing, enabled, calculated;
} sweep_t;

typedef struct noise
{
    timer_t timer;
    u8 shift;
    bool width_mode;
    u16 lfsr;
} noise_t;

typedef struct wave
{
    timer_t timer;
    u16 address, frequency;
    u8 buffer, position, shift;
} wave_t;

typedef struct envelope
{
    timer_t timer;
    u16 start_volume;
    u8 volume;
    i8 direction;
    bool enabled;
} envelope_t;

typedef struct channel
{
    envelope_t envelope;
    duty_t duty;
    u16 counter;
    bool length_enable, state, enabled, dac, left, right;
} channel_t;

typedef struct apu
{
    u16 sync_clock;
    i16 sample;
    u8 left_volume, right_volume;
    bool disabled, div_bit;
    channel_t ch1; /* tone & sweep */
    channel_t ch2; /* tone */
    channel_t ch3; /* wave output */
    channel_t ch4; /* noise */
} apu_t;

void apu_init(apu_t* apu);

i16 apu_tone_sweep(apu_t* apu, u32 seek, usize sample_rate);

i16 apu_sine_wave(u32 seek, usize sample_rate);
i16 apu_sawtooth_wave(u32 seek, usize sample_rate);
i16 apu_triangle_wave(u32 seek, usize sample_rate);
i16 apu_square_wave(u32 seek, usize sample_rate);

u8* apu_map(apu_t* apu, u16 address);
u8 apu_peek(apu_t* apu, u16 address);
void apu_poke(apu_t* apu, u16 address, u8 value);

#endif
