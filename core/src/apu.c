#include "core/apu.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const bool duty_table[4][8] = {
        { 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 1, 1, 1 },
        { 0, 1, 1, 1, 1, 1, 1, 0 }
};

void apu_init(apu_t* apu)
{

}

i16 apu_tone_sweep(apu_t* apu, u32 seek, usize sample_rate)
{
    return apu_triangle_wave(seek, sample_rate);
}

double apu_wave_gen(u32 seek, double pitch, usize sample_rate)
{
    double tpc = (double)sample_rate / pitch;
    double cycles = ((double)seek) / tpc;

    return (M_PI * 2.0) * cycles;
}

i16 apu_wave_form(double wave)
{
    double range = (pow(2, sizeof(i16) * 8) / 2.0) - 1.0;
    return wave * range;
}

i16 apu_sine_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = apu_wave_gen(seek, pitch, sample_rate);

    return apu_wave_form(sin(wave));
}

i16 apu_sawtooth_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = apu_wave_gen(seek, pitch, sample_rate);

    double t = (wave / M_PI) / 4.0;

    return apu_wave_form((4.0 * (t - floor(t + 0.5))) - 1.0);
}

i16 apu_triangle_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = apu_wave_gen(seek, pitch, sample_rate);

    double t = (wave / M_PI) / 2.0;
    double a = fabs(t - floor(t + 0.5));

    return apu_wave_form((4.0 * a) - 1.0);
}

i16 apu_square_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = apu_wave_gen(seek, pitch, sample_rate);

    return apu_wave_form(floor(sin(wave)) + 1.0);
}

u8* apu_map(apu_t* apu, u16 address)
{
    switch (address)
    {
//    case NR10:
//        break;
    default:
        printf("[!] unable to map mmu address `0x%04X`\n", address);
        exit(EXIT_FAILURE);
    }
}

u8 apu_peek(apu_t* apu, u16 address)
{
    return *apu_map(apu, address);
}

void apu_poke(apu_t* apu, u16 address, u8 value)
{
    *apu_map(apu, address) = value;
}
