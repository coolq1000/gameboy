#include "core/apu.h"

#include <stdlib.h>
#include <math.h>

void apu_create(apu_t* apu)
{
    apu->ch1.sample_callback = triangle_wave;
}

void apu_destroy(apu_t* apu)
{
	
}

double wave_gen(u32 seek, double pitch, usize sample_rate)
{
    double tpc = (double)sample_rate / pitch;
    double cycles = ((double)seek) / tpc;

    return (M_PI * 2.0) * cycles;
}

i16 wave_form(double wave)
{
    double range = (pow(2, sizeof(i16) * 8) / 2.0) - 1.0;
    return wave * range;
}

i16 sine_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = wave_gen(seek, pitch, sample_rate);

    return wave_form(sin(wave));
}

i16 sawtooth_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = wave_gen(seek, pitch, sample_rate);

    double t = (wave / M_PI) / 4.0;

    return wave_form((4.0 * (t - floor(t + 0.5))) - 1.0);
}

i16 triangle_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = wave_gen(seek, pitch, sample_rate);

    double t = (wave / M_PI) / 2.0;
    double a = fabs(t - floor(t + 0.5));

    return wave_form((4.0 * a) - 1.0);
}

i16 square_wave(u32 seek, usize sample_rate)
{
    static double pitch = 220.0;
    double wave = wave_gen(seek, pitch, sample_rate);

    return wave_form(floor(sin(wave)) + 1.0);
}
