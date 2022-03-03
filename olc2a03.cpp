#include "olc2a03.h"

olc2A03::olc2A03()
{
}

void olc2A03::cpuWrite(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0x4000:
        switch ((data & 0xC0) >> 6)
        {
        case 0x00: pulse1_seq.sequence = 0b01000000; pulse1_osc.dutycycle = 0.125; break;
        case 0x01: pulse1_seq.sequence = 0b01100000; pulse1_osc.dutycycle = 0.250; break;
        case 0x02: pulse1_seq.sequence = 0b01111000; pulse1_osc.dutycycle = 0.500; break;
        case 0x03: pulse1_seq.sequence = 0b10011111; pulse1_osc.dutycycle = 0.750; break;
        }
        break;
    case 0x4001:
        break;
    case 0x4002:
        pulse1_seq.reload = (pulse1_seq.reload & 0xFF00) | data;
        break;
    case 0x4003:
        pulse1_seq.reload = (uint16_t)((data & 0x07)) << 8 | (pulse1_seq.reload & 0x00FF);
        pulse1_seq.timer = pulse1_seq.reload;
        break;
    case 0x4004:
        break;
    case 0x4005:
        break;
    case 0x4006:
        break;
    case 0x4007:
        break;
    case 0x4008:
        break;
    case 0x400C:
        break;
    case 0x400E:
        break;
    case 0x4015:
        pulse1_enable = data & 0x01;
        break;
    case 0x400F:
        break;
    }
}

uint8_t olc2A03::cpuRead(uint16_t addr)
{
    return 0x00;
}

void olc2A03::clock()
{
    bool bQuarterFrameClock = false;
    bool bHalfFrameClock = false;

    dGlobalTime += (0.333333333 / 1789773);

    if (clock_counter % 6 == 0)
    {
        frame_clock_counter++;

        // 4-Step Sequence Mode
        if (frame_clock_counter == 3729)
        {
            bQuarterFrameClock = true;
        }

        if (frame_clock_counter == 7457)
        {
            bQuarterFrameClock = true;
            bHalfFrameClock = true;
        }

        if (frame_clock_counter == 11186)
        {
            bQuarterFrameClock = true;
        }

        if (frame_clock_counter == 14916)
        {
            bQuarterFrameClock = true;
            bHalfFrameClock = true;
            frame_clock_counter = 0;
        }

        // Update functional units

        // Quater frame "beats" adjust the volume envelope
        if (bQuarterFrameClock)
        {



            // Half frame "beats" adjust the note length and
            // frequency sweepers
            if (bHalfFrameClock)
            {

            }

            //        pulse1_seq.clock(pulse1_enable, [](uint32_t &s)
            //        {
            //            s = ((s & 0x0001) >> 7) | ((s & 0x00FE) >> 1);
            //        });

            pulse1_osc.frequency = 1789773.0 / (16.0 * (double)(pulse1_seq.reload + 1));
            pulse1_sample = pulse1_osc.sample(dGlobalTime);
        }

        clock_counter ++;
    }
};

    void olc2A03::reset()
    {

    }

    double olc2A03::GetOutputSample()
    {
        return pulse1_sample;
    }
