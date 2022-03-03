
#include <cstdint>
#include <array>
#include "olc6502.h"
#include "olc2c02.h"
#include "olc2a03.h"
#include "cartridge.h"


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

    //APU
    olc2A03 apu;

    // RAM
    std::array<uint8_t, 2048> cpuRam;

    // Cartridge
    std::shared_ptr<Cartridge> cart;

    uint8_t controller[2];

public:
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);

    void SetSampleFrequency(uint32_t sample_rate);

    double dAudioSample = 0.0;

public:
    void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
    void reset();
    void clock();

private:
    // Counts how many clock cycles have passed
    uint32_t nSystemClockCounter = 0;

    uint8_t controller_state[2];

    uint8_t dma_page = 0x00;
    uint8_t dma_addr = 0x00;
    uint8_t dma_data = 0x00;

    bool dma_transfer = false;
    bool dma_dummy = true;
};



