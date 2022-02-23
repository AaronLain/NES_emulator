#include "bus.h"
#include <stdint.h>
Bus::Bus() {
    cpu.ConnectBus(this);

    for (auto &i : cpuRam) i = 0x00;
}

Bus::~Bus() {

}

void Bus::cpuWrite(uint16_t addr, uint8_t data)
{
    if (addr >= 0x0000 && addr <= 0x1FFF)
    {
        cpuRam[addr & 0x07FF] = data;
    }
}

uint8_t Bus::cpuRead(uint16_t addr, bool bReadOnly)
{
    uint8_t data = 0x00;

    if (addr >= 0x0000 && addr <= 0x1FFF)
    {
        data = cpuRam[addr & 0x07FF];
    }

    return data;
}


