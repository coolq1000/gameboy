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

u8 apu_peek(apu_t* apu, u16 address)
{
    return 0xFA;
    switch (address)
    {
    case MMAP_IO_NR10: return apu->nr10;
    case MMAP_IO_NR11: return apu->nr11;
    case MMAP_IO_NR12: return apu->nr12;
    case MMAP_IO_NR13: return apu->nr13;
    case MMAP_IO_NR14: return apu->nr14;
    case MMAP_IO_NR21: return apu->nr21;
    case MMAP_IO_NR22: return apu->nr22;
    case MMAP_IO_NR23: return apu->nr23;
    case MMAP_IO_NR24: return apu->nr24;
    case MMAP_IO_NR30: return apu->nr30;
    case MMAP_IO_NR31: return apu->nr31;
    case MMAP_IO_NR32: return apu->nr32;
    case MMAP_IO_NR33: return apu->nr33;
    case MMAP_IO_NR34: return apu->nr34;
    case MMAP_IO_NR41: return apu->nr41;
    case MMAP_IO_NR42: return apu->nr42;
    case MMAP_IO_NR43: return apu->nr43;
    case MMAP_IO_NR44: return apu->nr44;
    case MMAP_IO_NR50: return apu->nr50;
    case MMAP_IO_NR51: return apu->nr51;
    case MMAP_IO_NR52: return apu->nr52;
    default:
        printf("[!] unable to map apu address `0x%04X`\n", address);
        exit(EXIT_FAILURE);
    }
}

void apu_poke(apu_t* apu, u16 address, u8 value)
{
    switch (address)
    {
        case MMAP_IO_NR10:
            break;
        case MMAP_IO_NR11:
            break;
        case MMAP_IO_NR12:
            break;
        case MMAP_IO_NR13:
            break;
        case MMAP_IO_NR14:
            break;
        case MMAP_IO_NR21:
            break;
        case MMAP_IO_NR22:
            break;
        case MMAP_IO_NR23:
            break;
        case MMAP_IO_NR24:
            break;
        case MMAP_IO_NR30:
            break;
        case MMAP_IO_NR31:
            break;
        case MMAP_IO_NR32:
            break;
        case MMAP_IO_NR33:
            break;
        case MMAP_IO_NR34:
            break;
        case MMAP_IO_NR41:
            break;
        case MMAP_IO_NR42:
            break;
        case MMAP_IO_NR43:
            break;
        case MMAP_IO_NR44:
            break;
        case MMAP_IO_NR50:
            break;
        case MMAP_IO_NR51:
            break;
        case MMAP_IO_NR52:
            break;
        default:
            printf("[!] unable to map apu address `0x%04X`\n", address);
            exit(EXIT_FAILURE);
    }
}
