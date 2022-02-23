
#include <cstdint>
#include "olc6502.h"
#include <array>

class Bus
{
public:
    Bus();
    virtual ~Bus();

public:
    // CPU
    olc6502 cpu;

    // PPU
    olc2C02 ppu;

    std::array<uint8_t, 2048> cpuRam;

public:
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);
};



