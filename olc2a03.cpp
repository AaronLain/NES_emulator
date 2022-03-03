#include "olc2a03.h"

olc2A03::olc2A03()
{
}

void olc2A03::cpuWrite(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0x4000:
        break;
    case 0x4001:
        break;
    case 0x4002:
        break;
    case 0x4003:
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

}

void olc2A03::reset()
{

}
