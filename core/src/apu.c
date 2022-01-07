#include "core/apu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8 duty_table[4][8] = {
        { 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 1, 1, 1 },
        { 0, 1, 1, 1, 1, 1, 1, 0 }
};

bool timer_tick(timer_t* timer)
{
    if (--timer->counter == 0)
    {
        timer_reset(timer);
        return true;
    }

    return false;
}

void timer_reset(timer_t* timer)
{
    timer->counter = timer->period;
}

void duty_cycle(duty_t* duty)
{
    duty->timer.period = (2048 - duty->frequency) * 4;

    if (timer_tick(&duty->timer))
    {
        duty->position++;
        duty->state = duty_table[duty->pattern][duty->position % 0x8];
    }
}

void envelope_cycle(envelope_t* envelope)
{
    if (envelope->enabled)
    {
        if (envelope->timer.period == 0) envelope->timer.period = 8;

        if (timer_tick(&envelope->timer))
        {
            envelope->volume += envelope->direction;
            if (envelope->volume == 0x10 || envelope->volume == 0xFF)
            {
                envelope->volume -= envelope->direction;
                envelope->enabled = false;
            }
        }
    }
}

void sweep_cycle(sweep_t* sweep, duty_t* duty, channel_t* channel)
{
    if (sweep->enabled)
    {
        if (timer_tick(&sweep->timer) && sweep->timer.period)
        {
            u16 sweep_frequency = sweep_frequency_calc(sweep);
            if (sweep->shift && sweep_frequency < 2048)
            {
                sweep->frequency = sweep_frequency;
                duty->frequency = sweep_frequency;
            }

            sweep_frequency = sweep_frequency_calc(sweep);
            if (sweep_frequency >= 2048) channel->enabled = false;
        }
    }
}

u16 sweep_frequency_calc(sweep_t* sweep)
{
    if (sweep->decreasing)
    {
        sweep->calculated = true;
        return sweep->frequency - (sweep->frequency >> sweep->shift);
    }

    return sweep->frequency + (sweep->frequency >> sweep->shift);
}

void wave_cycle(wave_t* wave, bus_t* bus)
{
    if (timer_tick(&wave->timer))
    {
        wave->position++;
        wave->output = bus_peek8(bus, MMAP_IO_WAVE + ((wave->position & 0x1F) / 2));
        if (wave->position % 2)
        {
            wave->output &= 0xF;
        }
        else
        {
            wave->output >>= 4;
        }
    }
}

void noise_cycle(noise_t* noise)
{
    if (timer_tick(&noise->timer))
    {
        u8 lfsr_low = noise->lfsr & 0xFF;
        u8 tmp = (lfsr_low & 0x1) ^ ((lfsr_low & 0x2) >> 1);
        noise->lfsr >>= 0x1;
        noise->lfsr &= 0xBFFF;
        noise->lfsr |= tmp * 0x4000;
        if (noise->width_mode)
        {
            noise->lfsr &= 0xFFBF;
            noise->lfsr |= tmp * 0x40;
        }
        noise->state = !(noise->lfsr & 0x1);
    }
}

void channel_length_cycle(channel_t* channel)
{
    channel->length.timer.counter--;
    if (channel->length.enabled && channel->length.timer.counter == 0) channel->enabled = false;
}

void apu_init(apu_t* apu, usize sample_rate, usize latency)
{
    *apu = (apu_t){ 0 };
    apu->sample_rate = sample_rate;
    apu->latency = latency;
    apu->buffer1 = malloc(sizeof(i16) * latency);
    apu->buffer2 = malloc(sizeof(i16) * latency);

    /* empty mixing buffer */
    memset(apu->buffer1, 0, sizeof(i16) * latency);
    memset(apu->buffer2, 0, sizeof(i16) * latency);
}

void apu_free(apu_t* apu)
{
    free(apu->buffer1);
    free(apu->buffer2);
}

void apu_cycle(apu_t* apu, bus_t* bus, usize cycles)
{
    if (apu->enabled)
    {
        apu->clock += cycles;
        apu->sample_clock += cycles;

        for (usize i = 0; i < cycles; i++)
        {
            if (apu->ch1.duty.enabled) duty_cycle(&apu->ch1.duty);
            if (apu->ch2.duty.enabled) duty_cycle(&apu->ch2.duty);
            wave_cycle(&apu->ch3.wave, bus);
            noise_cycle(&apu->ch4.noise);
        }

        /* output sample to buffer */
        usize cycles_per_sample = CPU_FREQUENCY / apu->sample_rate;
        if (apu->sample_clock >= cycles_per_sample)
        {
            i16 sample = 0;

            sample += apu_ch1_sample(apu);
            sample += apu_ch2_sample(apu);
            sample += apu_ch3_sample(apu);
            sample += apu_ch4_sample(apu);

            (apu->flip ? apu->buffer2 : apu->buffer1)[apu->sample++ % apu->latency] = sample;

            apu->flip = (apu->sample / apu->latency) % 2;

            apu->sample_clock -= cycles_per_sample;
        }
    }
}

void apu_frame_sequencer(apu_t* apu)
{
    /* length clock */
    if (!(apu->frame_sequence & 0x1))
    {
        if (apu->ch1.enabled) channel_length_cycle(&apu->ch1);
        if (apu->ch2.enabled) channel_length_cycle(&apu->ch2);
        if (apu->ch3.enabled) channel_length_cycle(&apu->ch3);
        if (apu->ch4.enabled) channel_length_cycle(&apu->ch4);
    }

    /* sweep clock */
    if (apu->frame_sequence == 0x2 || apu->frame_sequence == 0x6)
    {
        if (apu->ch3.enabled) sweep_cycle(&apu->ch1.sweep, &apu->ch1.duty, &apu->ch1);
    }

    /* envelope clock */
    if (apu->frame_sequence == 0x7)
    {
        envelope_cycle(&apu->ch1.envelope);
        envelope_cycle(&apu->ch2.envelope);
        envelope_cycle(&apu->ch4.envelope);
    }

    apu->frame_sequence = (apu->frame_sequence + 1) % 8;
}

void apu_ch1_trigger(apu_t* apu)
{
    apu->ch1.enabled = true;
    apu->ch1.duty.enabled = true;
    if (!apu->ch1.length.timer.counter) apu->ch1.length.timer.counter = 64;

    timer_reset(&apu->ch1.duty.timer);
    timer_reset(&apu->ch1.envelope.timer);
    timer_reset(&apu->ch1.sweep.timer);

    apu->ch1.envelope.enabled = apu->ch1.envelope.timer.period > 0;
    apu->ch1.envelope.volume = apu->ch1.envelope.start_volume;

    apu->ch1.sweep.enabled = apu->ch1.sweep.shift || apu->ch1.sweep.timer.period;
    apu->ch1.sweep.frequency = apu->ch1.duty.frequency;
    apu->ch1.sweep.calculated = false;

    if (apu->ch1.sweep.shift)
    {
        u16 sweep_frequency = sweep_frequency_calc(&apu->ch1.sweep);
        if (sweep_frequency >= 2048) apu->ch1.enabled = false;
    }

    if (!apu->ch1.dac) apu->ch1.enabled = false;
}

void apu_ch2_trigger(apu_t* apu)
{
    apu->ch2.enabled = true;
    apu->ch2.duty.enabled = true;
    if (!apu->ch2.length.timer.counter) apu->ch2.length.timer.counter = 64;

    timer_reset(&apu->ch2.duty.timer);
    timer_reset(&apu->ch2.envelope.timer);

    apu->ch2.envelope.enabled = apu->ch2.envelope.timer.period > 0;
    apu->ch2.envelope.volume = apu->ch2.envelope.start_volume;

    if (!apu->ch2.dac) apu->ch2.enabled = false;
}

void apu_ch3_trigger(apu_t* apu)
{
    apu->ch3.enabled = true;
    if (!apu->ch3.length.timer.counter) apu->ch3.length.timer.counter = 256;
    apu->ch3.wave.position = 0;

    timer_reset(&apu->ch3.wave.timer);

    if (!apu->ch3.dac) apu->ch3.enabled = false;
}

void apu_ch4_trigger(apu_t* apu)
{
    apu->ch4.enabled = true;

    timer_reset(&apu->ch4.noise.timer);
    timer_reset(&apu->ch4.envelope.timer);

    apu->ch4.envelope.enabled = apu->ch4.envelope.timer.period > 0;
    apu->ch4.envelope.volume = apu->ch4.envelope.start_volume;

    apu->ch4.noise.lfsr = 0xFFFF;

    if (!apu->ch4.dac) apu->ch4.enabled = false;
}

i16 apu_ch1_sample(apu_t* apu)
{
    i16 sample = 0;

    if (apu->ch1.dac)
    {
        sample -= AMP_CHL / 2;
    }
    if (apu->ch1.enabled)
    {
        sample += AMP_BASE * apu->ch1.duty.state * apu->ch1.envelope.volume;
    }

    return sample;
}

i16 apu_ch2_sample(apu_t* apu)
{
    i16 sample = 0;

    if (apu->ch2.dac)
    {
        sample -= AMP_CHL / 2;
    }
    if (apu->ch2.enabled)
    {
        sample += AMP_BASE * apu->ch2.duty.state * apu->ch2.envelope.volume;
    }

    return sample;
}

i16 apu_ch3_sample(apu_t* apu)
{
    i16 sample = 0;

    if (apu->ch3.dac)
    {
        sample -= AMP_CHL / 2;
    }
    if (apu->ch3.enabled && apu->ch3.wave.shift)
    {
        sample += AMP_BASE * (apu->ch3.wave.output >> (apu->ch3.wave.shift - 1));
    }

    return sample;
}

i16 apu_ch4_sample(apu_t* apu)
{
    i16 sample = 0;

    if (apu->ch4.dac)
    {
        sample -= AMP_CHL / 2;
    }
    if (apu->ch4.enabled)
    {
        sample += AMP_BASE * apu->ch4.noise.state * apu->ch4.envelope.volume;
    }

    return sample;
}

u8 apu_peek(apu_t* apu, u16 address)
{
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
    case MMAP_IO_NR52:
    {
        u8 ret = apu->nr52 & 0x70;
        ret |= apu->ch1.enabled << 0;
        ret |= apu->ch2.enabled << 1;
        ret |= apu->ch3.enabled << 2;
        ret |= apu->ch4.enabled << 3;
        ret |= apu->enabled << 7;
        return ret;
    }
    default:
        printf("[!] unable to read apu address `0x%04X`\n", address);
        exit(EXIT_FAILURE);
    }
}

void apu_poke(apu_t* apu, u16 address, u8 value)
{
    switch (address)
    {
        case MMAP_IO_NR10:
            apu->nr10 = value;
            apu->ch1.sweep.timer.period = (value & 0x70) >> 4;
//            if (!apu->ch1.sweep.timer.period) apu->ch1.sweep.timer.period = 8; /* this will always make it sweep? */
            if (value & BIT(3) && apu->ch1.sweep.decreasing && apu->ch1.sweep.calculated)
            {
                apu->ch1.enabled = false;
            }

            apu->ch1.sweep.decreasing = value & BIT(3);
            apu->ch1.sweep.shift = value & 0x7;
            break;
        case MMAP_IO_NR11:
            apu->nr11 = value;
            apu->ch1.duty.pattern = (value & 0xC0) >> 0x6;
            apu->ch1.length.timer.period = 64 - (value & 0x3F);
            break;
        case MMAP_IO_NR12:
            apu->nr12 = value;
            apu->ch1.dac = value & 0xF8;
            apu->ch1.envelope.start_volume = (value & 0xF0) >> 0x4;
            apu->ch1.envelope.direction = (value & 0x8) ? 1 : -1;
            apu->ch1.envelope.timer.period = value & 0x7;
            if (!apu->ch1.dac) apu->ch1.enabled = false;
            break;
        case MMAP_IO_NR13:
            apu->nr13 = value;
            apu->ch1.duty.frequency &= 0xFF00;
            apu->ch1.duty.frequency |= value;
            break;
        case MMAP_IO_NR14:
            apu->nr14 = value;
            apu->ch1.length.enabled = value & 0x40;
            apu->ch1.duty.frequency &= 0x00FF;
            apu->ch1.duty.frequency |= (value & 0x7) << 0x8;
            if (value & 0x80) apu_ch1_trigger(apu);
            break;
        case MMAP_IO_NR15:
            break; /* unused */
        case MMAP_IO_NR21:
            apu->nr21 = value;
            apu->ch2.duty.pattern = (value & 0xC0) >> 0x6;
            apu->ch2.length.timer.period = 64 - (value & 0x3F);
            break;
        case MMAP_IO_NR22:
            apu->nr22 = value;
            apu->ch2.dac = value & 0xF8;
            if (!apu->ch2.dac) apu->ch2.enabled = false;
            apu->ch2.envelope.start_volume = (value & 0xF0) >> 0x4;
            apu->ch2.envelope.direction = (value & 0x8) ? 1 : -1;
            apu->ch2.envelope.timer.period = value & 0x7;
            break;
        case MMAP_IO_NR23:
            apu->nr23 = value;
            apu->ch2.duty.frequency &= 0xFF00;
            apu->ch2.duty.frequency |= value;
            break;
        case MMAP_IO_NR24:
            apu->nr24 = value;
            apu->ch2.length.enabled = value & 0x40;
            apu->ch2.duty.frequency &= 0x00FF;
            apu->ch2.duty.frequency |= (value & 0x7) << 0x8;
            if (value & 0x80) apu_ch2_trigger(apu);
            break;
        case MMAP_IO_NR30:
            apu->nr30 = value;
            apu->ch3.dac = value >> 7;
            if (!apu->ch3.dac) apu->ch3.enabled = false;
            break;
        case MMAP_IO_NR31:
            apu->nr31 = value;
            apu->ch3.length.timer.period = 256 - value;
            break;
        case MMAP_IO_NR32:
            apu->nr32 = value;
            apu->ch3.wave.shift = (value & 0x60) >> 5;
            break;
        case MMAP_IO_NR33:
            apu->nr33 = value;
            apu->ch3.wave.frequency &= 0xFF00;
            apu->ch3.wave.frequency |= value;
            apu->ch3.wave.timer.period = (2048 - apu->ch3.wave.frequency) * 2;
            break;
        case MMAP_IO_NR34:
            apu->nr34 = value;
            apu->ch3.length.enabled = value & 0x40;
            apu->ch3.wave.frequency &= 0x00FF;
            apu->ch3.wave.frequency |= (value & 0x7) << 0x8;
            apu->ch3.wave.timer.period = (2048 - apu->ch3.wave.frequency) * 2;
            if (value & 0x80) apu_ch3_trigger(apu);
            break;
        case MMAP_IO_NR40:
            break; /* unused */
        case MMAP_IO_NR41:
            apu->nr41 = value;
            apu->ch4.length.timer.period = 64 - (value & 0x3F);
            break;
        case MMAP_IO_NR42:
            apu->nr42 = value;
            apu->ch4.dac = value & 0xF8;
            if (!apu->ch4.dac) apu->ch4.enabled = false;
            apu->ch4.envelope.start_volume = (value & 0xF0) >> 0x4;
            apu->ch4.envelope.direction = (value & 0x8) ? 1 : -1;
            apu->ch4.envelope.timer.period = value & 0x7;
            break;
        case MMAP_IO_NR43:
            apu->nr43 = value;
            apu->ch4.noise.shift = (value & 0xF0) >> 0x4;
            apu->ch4.noise.width_mode = value & 0x8;
            apu->ch4.noise.timer.period = (value & 0x7) << 4;
            if (apu->ch4.noise.timer.period == 0 ) apu->ch4.noise.timer.period = 8;
            apu->ch4.noise.timer.period <<= apu->ch4.noise.shift;
            break;
        case MMAP_IO_NR44:
            apu->nr44 = value;
            apu->ch4.length.enabled = value & 0x40;
            if (value & 0x80) apu_ch4_trigger(apu);
            break;
        case MMAP_IO_NR50:
            apu->nr50 = value;
            break;
        case MMAP_IO_NR51:
            apu->nr51 = value;
            apu->ch1.right  = value & 0x01;
            apu->ch2.right  = value & 0x02;
            apu->ch3.right  = value & 0x04;
            apu->ch4.right  = value & 0x08;
            apu->ch1.left   = value & 0x10;
            apu->ch2.left   = value & 0x20;
            apu->ch3.left   = value & 0x40;
            apu->ch4.left   = value & 0x80;
            break;
        case MMAP_IO_NR52:
            apu->nr52 = value;
            apu->enabled = value & 0x80;
            if (!apu->enabled) apu_init(apu, apu->sample_rate, apu->latency);
            break;
        default:
            printf("[!] unable to write apu address `0x%04X`\n", address);
            exit(EXIT_FAILURE);
    }
}
