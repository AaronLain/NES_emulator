#pragma once

#include <cstdint>
#include <functional>


class olc2A03
{
public:
    olc2A03();


public:
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr);
    void clock();
    void reset();

    double GetOutputSample();

private:

    struct sequencer
    {
        uint32_t sequence = 0x00000000;
        uint16_t timer = 0x0000;
        uint16_t reload = 0x0000;
        uint8_t output = 0x00;

        uint8_t clock(bool bEnable, std::function<void(uint32_t &s)> funcManip)
        {
            if(bEnable)
            {
                timer--;
                if (timer == 0xFFFF)
                {
                    timer = reload + 1;
                    funcManip(sequence);
                    output = sequence & 0x00000001;
                }
            }

            return output;
        }
    };

    bool pulse1_enable = false;
    double pulse1_sample = 0.0;
};


