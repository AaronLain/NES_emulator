#include "olc2c02.h"

olc2C02::olc2C02()
{

}

uint8_t olc2C02::cpuRead(uint16_t addr, bool rdonly)
{
    uint8_t data = 0x00;

    switch (addr)
    {
    case 0x0000: // Control
        break;
    case 0x0001: // Mask
        break;
    case 0x0002: // Status
        data = (status.reg & 0xE0) | (ppu_data_buffer & 0x1F);
        status.vertical_blank = 0;
        address_latch = 0;
        break;
    case 0x0003: // OAM Address
        break;
    case 0x0004: // OAM Data
        break;
    case 0x0005: // Scroll
        break;
    case 0x0006: // PPU Address
        break;
    case 0x0007: // PPU Data
        data = ppu_data_buffer;
        ppu_data_buffer = ppuRead(ppu_address);

        if (ppu_address > 0x3f00) data = ppu_data_buffer;
        ppu_address++;
        break;
    }

    return data;
}

void olc2C02::cpuWrite(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0x0000: // Control
        control.reg = data;
        break;
    case 0x0001: // Mask
        mask.reg = data;
        break;
    case 0x0002: // Status
        break;
    case 0x0003: // OAM Address
        break;
    case 0x0004: // OAM Data
        break;
    case 0x0005: // Scroll
        break;
    case 0x0006: // PPU Address
        if (address_latch == 0)
        {
            ppu_address = (ppu_address & 0x00FF) | (data << 8);
            address_latch = 1;
        }
        else
        {
            ppu_address = (ppu_address & 0xFF00) | data;
            address_latch = 0;
        }
        break;
    case 0x0007: // PPU Data
        ppuWrite(ppu_address, data);
        break;
    }
}

void olc2C02::ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
    this->cart = cartridge;
}

void olc2C02::clock()
{
    if (scanline == -1 && cycle == 0)
    {
        status.vertical_blank = 0;
    }

    if (scanline == 241 && cycle == 1)
    {
        status.vertical_blank = 1;
        if (control.enable_nmi)
        {
            nmi = true;
        }
    }

    cycle++;
    if (cycle >= 341)
    {
        cycle = 0;
        scanline++;
        if (scanline >= 261)
        {
            scanline = -1;
            frame_complete = true;
        }
    }
}

uint8_t olc2C02::ppuRead(uint16_t addr, bool rdonly)
{
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if (cart->ppuRead(addr, data))
    {
        printf("Address %d", addr);
    }

    return data;
}

void olc2C02::ppuWrite(uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;
}

