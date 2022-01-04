
#ifndef APU_H
#define APU_H

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

#define MMAP_IO_WAVE_START  0xFF30
#define MMAP_IO_WAVE_SIZE   0x10
#define MMAP_IO_WAVE_END    (MMAP_IO_WAVE_START + MMAP_IO_WAVE_SIZE)

#define AMP_MAX     (I16_MAX / 0x10)
#define AMP_CHL     (AMP_MAX / 0x04)
#define AMP_BASE    (AMP_CHL / 0x10)

typedef struct apu apu_t;

typedef struct timer
{
    u16 period, counter;
} timer_t;

void timer_reset(timer_t* timer);
bool timer_cycle(timer_t* timer);

typedef struct duty
{
    timer_t timer;
    u8 counter, cycle;
    u16 frequency;
    bool enabled;
} duty_t;

bool duty_cycle(duty_t* duty);

typedef struct sweep
{
    timer_t timer;
    u16 period, frequency, shift;
    bool decreasing, enabled, calculated;
} sweep_t;

u16 sweep_frequency_calc(sweep_t* sweep);

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

void envelope_cycle(envelope_t* envelope);

typedef struct channel
{
    envelope_t envelope;
    duty_t duty;
    u16 counter;
    bool length_enable, state, enabled, dac, left, right;
} channel_t;

void channel_length_counter_cycle(channel_t* channel);

typedef struct apu
{
    /* registers */
    u8 nr10, nr11, nr12, nr13, nr14;    /* channel 1 */
    u8 nr20, nr21, nr22, nr23, nr24;    /* channel 2 */
    u8 nr30, nr31, nr32, nr33, nr34;    /* channel 3 */
    u8 nr40, nr41, nr42, nr43, nr44;    /* channel 4 */
    u8 nr50, nr51, nr52;                /* mixer     */

    usize sample_rate, latency;
    u16 sync_clock;
    i16 sample;
    u8 left_volume, right_volume;
    bool enabled, div_bit;
    channel_t ch1; /* tone & sweep */
    channel_t ch2; /* tone */
    channel_t ch3; /* wave output */
    channel_t ch4; /* noise */
    sweep_t sweep;
    noise_t noise;
    wave_t wave;
    u8 sequence;
    i16* buffer;
    usize index;
} apu_t;

void apu_init(apu_t* apu, usize sample_rate, usize latency);
void apu_free(apu_t* api);

void apu_cycle(apu_t* apu);
void apu_update(apu_t* apu);
void apu_sequence_cycle(apu_t* apu);

void apu_ch1_trigger(apu_t* apu);

i16 apu_ch1_sample(apu_t* apu);

u8 apu_peek(apu_t* apu, u16 address);
void apu_poke(apu_t* apu, u16 address, u8 value);

#endif
