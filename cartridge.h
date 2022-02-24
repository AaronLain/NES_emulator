#ifndef CARTRIDGE_H
#define CARTRIDGE_H


#include <stdint.h>
#include <vector>

class Cartridge
{
public:
    Cartridge(const std::string& sFileName);
    ~Cartridge();

private:
    std::vector<uint8_t> vPRGMemory;
    std::vector<uint8_t> vCHRMemory;

    uint8_t nMapperID = 0;
    uint8_t nPRGBanks = 0;
    uint8_t nCHRBanks = 0;

public:
    //Main Bus
    bool cpuRead(uint16_t addr, uint8_t &data);
    bool cpuWrite(uint16_t addr, uint8_t data);

    //PPU Bus
    bool ppuRead(uint16_t addr, uint8_t &data);
    bool ppuWrite(uint16_t addr, uint8_t data);
};

#endif // CARTRIDGE_H
