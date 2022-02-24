#ifndef OLC2C02_H
#define OLC2C02_H

#include <stdio.h>
#include <stdint.h>
#include <cartridge.h>
#include <memory>

class olc2C02
{
public:
    olc2C02();
    ~olc2C02();

private:
    uint8_t tblName[2][1024];
    uint8_t tblPalette[32];
    uint8_t tblPattern[2][4096]; // not quite necessary

public:
    // Main Bus
    uint8_t cpuRead(uint16_t addr, bool rdonly = false);
    void    cpuWrite(uint16_t addr, uint8_t data);

    // PPU Bus
    uint8_t ppuRead(uint16_t addr, bool rdonly = false);
    void    ppuWrite(uint16_t addr, uint8_t data);

    bool frame_complete = false;

private:
    std::shared_ptr<Cartridge> cart;

private:
    int16_t scanline = 0;
    int16_t cycle = 0;

public:
    // Interface
    void ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge);
    void clock();
};
#endif // OLC2C02_H
