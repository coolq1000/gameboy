
#ifndef APU_H
#define APU_H

#include "bus.h"
#include "util.h"

#define MMAP_IO_NR10 0xFF10
#define MMAP_IO_NR11 0xFF11
#define MMAP_IO_NR12 0xFF12
#define MMAP_IO_NR13 0xFF13
#define MMAP_IO_NR14 0xFF14
#define MMAP_IO_NR15 0xFF15
#define MMAP_IO_NR21 0xFF16
#define MMAP_IO_NR22 0xFF17
#define MMAP_IO_NR23 0xFF18
#define MMAP_IO_NR24 0xFF19
#define MMAP_IO_NR30 0xFF1A
#define MMAP_IO_NR31 0xFF1B
#define MMAP_IO_NR32 0xFF1C
#define MMAP_IO_NR33 0xFF1D
#define MMAP_IO_NR34 0xFF1E
#define MMAP_IO_NR40 0xFF1F
#define MMAP_IO_NR41 0xFF20
#define MMAP_IO_NR42 0xFF21
#define MMAP_IO_NR43 0xFF22
#define MMAP_IO_NR44 0xFF23
#define MMAP_IO_NR50 0xFF24
#define MMAP_IO_NR51 0xFF25
#define MMAP_IO_NR52 0xFF26

#define MMAP_IO_WAVE 0xFF30

#define CPU_FREQUENCY 4190000

#define AMP_MAX     (I16_MAX)
#define AMP_CHL     (AMP_MAX / 0x04)
#define AMP_BASE    (AMP_CHL / 0x10)

/*
 * timer - a simple timer struct which counts down from the period
 */

typedef struct timer
{
    u16 counter, period;
} timer_t;

bool timer_tick(timer_t* timer);
void timer_reset(timer_t* timer);

/*
 * length - a timer with an enable switch
 */

typedef struct length
{
    timer_t timer;
    bool enabled;
} length_t;

bool length_tick(length_t* length);
void length_reset(length_t* length);

/*
 * duty - contains a timer which ticks at a frequency, outputs from the duty table
 */

typedef struct duty
{
    timer_t timer;
    u8 pattern, position;
    u16 frequency;
    bool enabled, state;
} duty_t;

u8 duty_cycle(duty_t* duty, usize cycles);

typedef struct envelope
{
    timer_t timer;
    u8 start_volume, volume;
    i8 direction;
    bool enabled;
} envelope_t;

void envelope_cycle(envelope_t* envelope);

typedef struct sweep
{
    u16 frequency;
    u8 shift;
    bool decreasing;
} sweep_t;

void sweep_cycle(sweep_t* sweep);

typedef struct wave
{
    timer_t timer;
    u16 frequency;
    u8 shift;
    u16 position;
    u8 output;
} wave_t;

u8 wave_cycle(wave_t* wave, bus_t* bus, usize cycles);

typedef struct channel
{
    bool dac, enabled, left, right;

    length_t length;
    duty_t duty;
    envelope_t envelope;
    sweep_t sweep;
    wave_t wave;
} channel_t;

void channel_length_cycle(channel_t* channel);

typedef struct apu
{
    /* audio variables */
    usize sample_rate, latency;

    /* registers */
    u8 nr10, nr11, nr12, nr13, nr14;    /* channel 1 */
    u8 nr20, nr21, nr22, nr23, nr24;    /* channel 2 */
    u8 nr30, nr31, nr32, nr33, nr34;    /* channel 3 */
    u8 nr40, nr41, nr42, nr43, nr44;    /* channel 4 */
    u8 nr50, nr51, nr52;                /* mixer     */

    bool enabled;
    channel_t ch1; /* tone & sweep */
    channel_t ch2; /* tone */
    channel_t ch3; /* wave output */
    channel_t ch4; /* noise */

    /* timing */
    u16 clock, sample_clock;

    /* frame sequencer */
    u8 frame_sequence;

    /* mixing */
    usize sample;
    i16* buffer;
} apu_t;

void apu_init(apu_t* apu, usize sample_rate, usize latency);
void apu_free(apu_t* apu);

void apu_cycle(apu_t* apu, bus_t* bus, usize cycles);
void apu_frame_sequencer(apu_t* apu);

void apu_ch1_trigger(apu_t* apu);
void apu_ch2_trigger(apu_t* apu);
void apu_ch3_trigger(apu_t* apu);

i16 apu_ch1_sample(apu_t* apu);
i16 apu_ch2_sample(apu_t* apu);
i16 apu_ch3_sample(apu_t* apu);

u8 apu_peek(apu_t* apu, u16 address);
void apu_poke(apu_t* apu, u16 address, u8 value);

#endif
