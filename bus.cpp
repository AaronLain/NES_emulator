#include "bus.h"
#include <stdint.h>
Bus::Bus() {
    cpu.ConnectBus(this);

    for (auto &i : cpuRam) i = 0x00;
}

Bus::~Bus()
{

}

void Bus::SetSampleFrequency(uint32_t sample_rate)
{
    double dAudioTimePerSystemSample = 1.0 / (double)sample_rate;
    double dAudioTimePerNESClock = 1.0 / 5369318.0;
}

void Bus::cpuWrite(uint16_t addr, uint8_t data)
{
    if (cart->cpuWrite(addr, data))
    {

    }
    else if (addr >= 0x0000 && addr <= 0x1FFF)
    {
        cpuRam[addr & 0x07FF] = data;
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        ppu.cpuWrite(addr & 0x007, data);
    }
    else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017)
    {
        apu.cpuWrite(addr, data);
    }
    else if (addr == 0x4014)
    {
        dma_page = data;
        dma_addr = 0x00;
        dma_transfer = true;
    }
    else if (addr >= 0x4016 && addr <= 0x4017)
    {
        controller_state[addr & 0x0001] = controller[addr & 0x0001];
    }
}

uint8_t Bus::cpuRead(uint16_t addr, bool bReadOnly)
{
    uint8_t data = 0x00;

    if (addr >= 0x0000 && addr <= 0x1FFF)
    {
        data = cpuRam[addr & 0x07FF];
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        ppu.cpuRead(addr & 0x007, data);
    }
    else if (addr >= 0x4016 && addr <= 0x4017)
    {
        data = (controller_state[addr <= 0x0001] & 0x80) > 0;
        controller_state[addr & 0x0001] <<= 1;
    }

    return data;
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge> &cartridge)
{
    this->cart = cartridge;
    ppu.ConnectCartridge(cartridge);
}

void Bus::reset()
{
    cpu.reset();
    nSystemClockCounter = 0;
}

bool Bus::clock()
{
    ppu.clock();

    apu.clock();

    // if DMA transfer requested:
    // on odd clock cycles read from CPU memory
    // on even clock cycles write to OAM
    if (nSystemClockCounter % 3 == 0)
    {
        if (dma_transfer)
        {
            if (dma_dummy)
            {
                if (nSystemClockCounter % 2 == 1)
                {
                    dma_dummy = false;
                }
            }
            else
            {
                if (nSystemClockCounter % 2 == 0)
                {
                    dma_data = cpuRead(dma_page << 8 | dma_addr);
                }
                else
                {
                    ppu.pOAM[dma_addr] = dma_data;
                    dma_addr++;

                    if (dma_addr == 0x00)
                    {
                        dma_transfer = false;
                        dma_dummy = true;
                    }
                }
            }
        }
        else
        {
            cpu.clock();
        }
    }

    bool bAudioSampleReady = false;
    dAudioTime += dAudioTimePerNESClock;
    if (dAudioTime >= dAudioTimesPerSystemSample)
    {
        dAudioTime -= dAudioTimesPerSystemSample;
        dAudioSample = apu.GetOutputSample();
    }

    if (ppu.nmi)
    {
        ppu.nmi = false;
        cpu.nmi();
    }

    nSystemClockCounter++;

    return bAudioSampleReady;
}

