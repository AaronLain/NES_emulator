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
    bool pulse1_enable = false;
    double pulse1_sample = 0.0;
};


