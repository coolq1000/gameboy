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

void timer_reset(timer_t* timer)
{
    timer->counter = timer->period;
}

bool timer_cycle(timer_t* timer)
{
    if (--timer->counter == 0)
    {
        timer_reset(timer);
        return true;
    }

    return false;
}

bool duty_cycle(duty_t* duty)
{
    duty->timer.period = (0x800 - duty->frequency) << 2;

    if (timer_cycle(&duty->timer))
    {
        duty->counter++;
        duty->counter %= 0x8;
    }

    return duty_table[duty->cycle][duty->counter];
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

void envelope_cycle(envelope_t* envelope)
{
    if (envelope->enabled)
    {
        if (envelope->timer.period == 0) envelope->timer.period = 8;

        if (timer_cycle(&envelope->timer))
        {
            envelope->volume += envelope->direction;
            if (envelope->volume == 0x10 || envelope->volume == 0xFF)
            {
                envelope->volume -= envelope->direction;
                envelope->enabled = true;
            }
        }
    }
}

void channel_length_counter_cycle(channel_t* channel)
{
    channel->counter--;
    if (channel->counter == 0)
    {
        channel->enabled = false;
    }
}

void apu_init(apu_t* apu)
{
    *apu = (apu_t){ 0 };
    apu->wave.address = MMAP_IO_WAVE_START;
    apu->wave.position = 0;
}

void apu_cycle(apu_t* apu)
{
    if (apu->enabled)
    {
        /* duty cycle */
        if (apu->ch1.duty.enabled) apu->ch1.state = duty_cycle(&apu->ch1.duty);
    }
}

void apu_sequence_cycle(apu_t* apu)
{
    /* length clocks */
    if (!(apu->sequence & 0x1))
    {
        if (apu->ch1.enabled && apu->ch1.length_enable)
        {
            channel_length_counter_cycle(&apu->ch1);
        }
        if (apu->ch2.enabled && apu->ch2.length_enable)
        {
            channel_length_counter_cycle(&apu->ch2);
        }
        if (apu->ch3.enabled && apu->ch3.length_enable)
        {
            channel_length_counter_cycle(&apu->ch3);
        }
        if (apu->ch4.enabled && apu->ch4.length_enable)
        {
            channel_length_counter_cycle(&apu->ch4);
        }
    }

    /* sweep clocks */
    if (apu->sweep.enabled && (apu->sequence == 0x2 || apu->sequence == 0x6))
    {
        if (timer_cycle(&apu->sweep.timer) && apu->sweep.period != 0)
        {
            u16 frequency = sweep_frequency_calc(&apu->sweep);
            if (apu->sweep.shift != 0 && frequency < 0x800)
            {
                apu->sweep.frequency = frequency;
                apu->ch1.duty.frequency = frequency;
                apu->nr13 = frequency & 0xFF;
                apu->nr14 &= ~0x7;
                apu->nr14 |= (frequency & 0x700) >> 0x8;
            }

            frequency = sweep_frequency_calc(&apu->sweep);
            if (frequency >= 0x800)
            {
                apu->ch1.enabled = false;
            }
        }
    }

    /* envelope clocks */
    if (apu->sequence == 0x7)
    {
        envelope_cycle(&apu->ch1.envelope);
        envelope_cycle(&apu->ch2.envelope);
        envelope_cycle(&apu->ch4.envelope);
    }

    apu->sequence++;
    apu->sequence &= 0x7;
}

void apu_ch1_trigger(apu_t* apu)
{
    apu->ch1.duty.enabled = true;
    apu->ch1.duty.cycle = (apu->nr11 & 0xC0) >> 0x6;
    apu->ch1.duty.frequency = apu->nr13;
    apu->ch1.duty.frequency = (apu->nr14 & 0x7) << 0x8;
    apu->ch1.enabled = true;

    if (apu->ch1.counter == 0) apu->ch1.counter = 64;

    timer_reset(&apu->ch1.duty.timer);

    apu->ch1.envelope.enabled = apu->ch1.envelope.timer.period > 0;
    apu->ch1.envelope.volume = apu->ch1.envelope.start_volume;
    timer_reset(&apu->ch1.envelope.timer);

    apu->sweep.calculated = false;
    apu->sweep.frequency = apu->ch1.duty.frequency;
    timer_reset(&apu->sweep.timer);

    apu->sweep.enabled = apu->sweep.shift != 0 || apu->sweep.period != 0;

    if (apu->sweep.shift != 0)
    {
        if (sweep_frequency_calc(&apu->sweep) >= 0x800)
        {
            apu->ch1.enabled = false;
        }
    }

    if (!apu->ch1.dac) apu->ch1.enabled = false;
}

i16 apu_ch1_sample(apu_t* apu)
{
    i16 sample = 0;

    if (apu->ch1.dac)
    {
//        sample -= AMP_CHL / 2;
    }
    if (apu->ch1.enabled)
    {
//        sample += AMP_BASE * apu->ch1.state * apu->ch1.envelope.volume;
        sample = apu->ch1.state * AMP_MAX;
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
            apu->sweep.period = (value & 0x70) >> 4;
            apu->sweep.timer.period = apu->sweep.period;
            if (apu->sweep.timer.period == 0) apu->sweep.timer.period = 8;
            if (!(value & 0x4) && apu->sweep.decreasing && apu->sweep.calculated) apu->ch1.enabled = false;
            apu->sweep.decreasing = value & 0x4;
            apu->sweep.shift = value & 0x7;
            break;
        case MMAP_IO_NR11:
            apu->nr11 = value;
            apu->ch1.duty.cycle = (value & 0xC0) >> 0x6;
            apu->ch1.counter = 0x40 - (value & 0x3F);
            break;
        case MMAP_IO_NR12:
            apu->nr12 = value;
            apu->ch1.dac = value & 0xF8;
            if (!apu->ch1.dac) apu->ch1.enabled = false;
            apu->ch1.envelope.start_volume = (value & 0xF0) >> 4;
            apu->ch1.envelope.direction = (value & 0x8) ? 1 : - 1;
            apu->ch1.envelope.timer.period = value & 0x7;
            break;
        case MMAP_IO_NR13:
            apu->nr13 = value;
            apu->ch1.duty.frequency &= ~0x00FF;
            apu->ch1.duty.frequency |= value;
            break;
        case MMAP_IO_NR14:
            apu->nr14 = value;
            apu->ch1.length_enable = value & 0x40;
            apu->ch1.duty.frequency &= ~0xFF00;
            apu->ch1.duty.frequency |= (value & 0x7) << 8;
            if (value & 0x80) apu_ch1_trigger(apu);
            break;
        case MMAP_IO_NR15:
            break; /* unused */
        case MMAP_IO_NR21:
            apu->nr21 = value;
            break;
        case MMAP_IO_NR22:
            apu->nr22 = value;
            break;
        case MMAP_IO_NR23:
            apu->nr23 = value;
            break;
        case MMAP_IO_NR24:
            apu->nr24 = value;
            break;
        case MMAP_IO_NR30:
            apu->nr30 = value;
            apu->ch3.dac = value >> 7;
            if (!apu->ch3.dac) apu->ch3.enabled = false;
            break;
        case MMAP_IO_NR31:
            apu->nr31 = value;
            break;
        case MMAP_IO_NR32:
            apu->nr32 = value;
            break;
        case MMAP_IO_NR33:
            apu->nr33 = value;
            break;
        case MMAP_IO_NR34:
            apu->nr34 = value;
            break;
        case MMAP_IO_NR40:
            break; /* unused */
        case MMAP_IO_NR41:
            apu->nr41 = value;
            break;
        case MMAP_IO_NR42:
            apu->nr42 = value;
            break;
        case MMAP_IO_NR43:
            apu->nr43 = value;
            break;
        case MMAP_IO_NR44:
            apu->nr44 = value;
            break;
        case MMAP_IO_NR50:
            apu->nr50 = value;
            apu->left_volume = (value & 0x70) >> 4;
            apu->right_volume = value & 0x7;
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
            if (!apu->enabled) apu_init(apu);
            break;
        default:
            printf("[!] unable to write apu address `0x%04X`\n", address);
            exit(EXIT_FAILURE);
    }
}
