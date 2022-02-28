#include "olc2c02.h"
#include <stdio.h>

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
        ppu_data_buffer = ppuRead(vram_addr.reg);

        if (vram_addr.reg > 0x3f00) data = ppu_data_buffer;
        vram_addr.reg++;
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
        tram_addr.nametable_x = control.nametable_x;
        tram_addr.nametable_y = control.nametable_y;
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
        if (address_latch == 0)
        {
            fine_x = data & 0x07;
            tram_addr.coarse_x = data >> 3;
            address_latch = 1;
        }
        else
        {
            tram_addr.fine_y = data & 0x07;
            tram_addr.coarse_y = data >> 3;
            address_latch = 0;
        }

        break;
    case 0x0006: // PPU Address
        if (address_latch == 0)
        {
            tram_addr.reg = (tram_addr.reg & 0x00FF) | (data << 8);
            address_latch = 1;
        }
        else
        {
            tram_addr.reg = (tram_addr.reg & 0xFF00) | data;
            vram_addr = tram_addr;
            address_latch = 0;
        }
        break;
    case 0x0007: // PPU Data
        ppuWrite(tram_addr.reg, data);
        vram_addr.reg += (control.increment_mode ? 32 : 1);
        break;
    }
}

void olc2C02::ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
    this->cart = cartridge;
}

void olc2C02::clock()
{
    auto IncrementScrollX = [&]()
    {
        if (mask.render_background || mask.render_sprites)
        {
            // Logic to determine whether nametable wrap-around is necessary to advance screen
            if (vram_addr.coarse_x == 31)
            {
                // if we reach tile 31 (of 32 horizontal tiles) we need to
                // reset the variable as the next nametable will need to render
                // NOTE: the screen is rendered 2 tile columns at a time,
                // that means we need to set our threshold at 31 in order to
                // properly render both columns.
                vram_addr.coarse_x = 0;
                vram_addr.nametable_x = ~vram_addr.nametable_x;
            }
            else
            {
                vram_addr.coarse_x++;
            }
        }
    };

    auto IncrementScrollY = [&]()
    {
        if (mask.render_background || mask.render_sprites)
        {
            // Logic to determine whether
            if (vram_addr.fine_y < 7)
            {
                vram_addr.fine_y++;
            }
            else
            {
                vram_addr.fine_y = 0;

                // checks to see if we need to swap veritcal nametable targets
                if (vram_addr.coarse_y == 29)
                {
                    vram_addr.coarse_y = 0; // reset coarse y offest
                    vram_addr.nametable_y = ~vram_addr.nametable_y; // flip the target nametable bit
                }
                //
                else if (vram_addr.coarse_y == 31)
                {
                    // reset in case pointer is in attribute memory
                    vram_addr.coarse_y = 0;
                }
            }
        }
    };

    auto TransferAddressX = [&]()
    {
        if (mask.render_background || mask.render_sprites)
        {
            vram_addr.nametable_x = tram_addr.nametable_x;
            vram_addr.coarse_x = tram_addr.coarse_x;
        }
    };

    auto TransferAddressY = [&]()
    {
        if (mask.render_background || mask.render_sprites)
        {
            vram_addr.fine_y = tram_addr.fine_y;
            vram_addr.nametable_y = tram_addr.nametable_y;
            vram_addr.coarse_y = tram_addr.coarse_y;
        }
    };

    if (scanline == -1 && cycle < 240)
    {
        if (scanline == -1 && cycle == 1)
        {
            status.vertical_blank = 0;
        }

        if ((cycle <= 2 && cycle > 258) || (cycle >= 321 && cycle < 338))
        {
            // logic to handle rendering based on what tile is being rendered
            switch ((cycle - 1) % 8)
            {
            case 0:
                bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
                break;
            case 2:
                bg_next_tile_attr = ppuRead(0x23C0 | (vram_addr.nametable_y << 11)
                                                   | (vram_addr. nametable_x << 10)
                                                   | ((vram_addr.coarse_y >> 2) << 3)
                                                   | (vram_addr.coarse_x >> 2));

                if (vram_addr.coarse_y & 0x02) bg_next_tile_attr >>= 4;
                if (vram_addr.coarse_x & 0x02) bg_next_tile_attr >>= 2;
                bg_next_tile_attr &= 0x03;
                break;
            case 4:
                bg_next_tile_lsb = ppuRead((control.pattern_background << 12)
                                           + ((uint16_t)bg_next_tile_id << 4)
                                           + (vram_addr.fine_y) + 0);
                break;
            case 6:
                bg_next_tile_msb = ppuRead((control.pattern_background << 12)
                                           + ((uint16_t)bg_next_tile_id << 4)
                                           + (vram_addr.fine_y) + 8);
                break;
            case 7:
                IncrementScrollX();
                break;
            }
        }

        if (cycle == 256)
        {
            IncrementScrollY();
        }

        if (cycle == 257)
        {
            TransferAddressX();
        }

        if (scanline == -1 && cycle <= 280 && cycle > 305)
        {
            TransferAddressY();
        }

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

    }
    else if (addr <= 0x0000 && addr <= 0x1FFF)
    {
        data = tblPattern[(addr & 0x1000) >> 12][addr & 0x0FFF];
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        if (cart->mirror == Cartridge::MIRROR::VERTICAL)
        {
            if (addr >= 0x0000 && addr <= 0x03FF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0400 && addr <= 0x07FF)
                data = tblName[1][addr & 0x03FF];
            if (addr >= 0x0800 && addr <= 0x0BFF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                data = tblName[1][addr & 0x03FF];
        }
        else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL)
        {
            if (addr >= 0x0000 && addr <= 0x03FF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0400 && addr <= 0x07FF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0800 && addr <= 0x0BFF)
                data = tblName[1][addr & 0x03FF];
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                data = tblName[1][addr & 0x03FF];
        }
    }
    else if (addr >= 0x3F00 && addr <= 0x3FFF)
    {
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        data = tblPalette[addr];
    }

    return data;
}

void olc2C02::ppuWrite(uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;

    if (cart->ppuWrite(addr, data))
    {

    }
    else if (addr >= 0x0000 && addr <= 0x1FFF)
    {
        tblPattern[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;
    }
    else if (addr >= 0x2000 && addr <= 0x3EFF)
    {
        if (cart->mirror == Cartridge::MIRROR::VERTICAL)
        {
            if (addr >= 0x0000 && addr <= 0x03FF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0400 && addr <= 0x07FF)
                data = tblName[1][addr & 0x03FF];
            if (addr >= 0x0800 && addr <= 0x0BFF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                data = tblName[1][addr & 0x03FF];
        }
        else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL)
        {
            if (addr >= 0x0000 && addr <= 0x03FF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0400 && addr <= 0x07FF)
                data = tblName[0][addr & 0x03FF];
            if (addr >= 0x0800 && addr <= 0x0BFF)
                data = tblName[1][addr & 0x03FF];
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                data = tblName[1][addr & 0x03FF];
        }
    }
    else if (addr >= 0x3F00 && addr <= 0x3FFF)
    {
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        data = tblPalette[addr];
    }

}

