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
        data= pOAM[oam_addr];
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


//void olc2C02::reset()
//{
//    fine_x = 0x00;
//    address_latch = 0x00;
//    ppu_data_buffer = 0x00;
//    scanline = 0;
//    cycle = 0;
//    bg_next_tile_id = 0x00;
//    bg_next_tile_attrib = 0x00;
//    bg_next_tile_lsb = 0x00;
//    bg_next_tile_msb = 0x00;
//    bg_shifter_pattern_lo = 0x0000;
//    bg_shifter_pattern_hi = 0x0000;
//    bg_shifter_attrib_lo = 0x0000;
//    bg_shifter_attrib_hi = 0x0000;
//    status.reg = 0x00;
//    mask.reg = 0x00;
//    control.reg = 0x00;
//    vram_addr.reg = 0x0000;
//    tram_addr.reg = 0x0000;
//}

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

    auto LoadBackgroundShifters = [&]()
    {
        bg_shifter_pattern_lo = (bg_shifter_pattern_lo & 0xFF00) | bg_next_tile_lsb;
        bg_shifter_pattern_hi = (bg_shifter_pattern_hi & 0xFF00) | bg_next_tile_msb;

        bg_shifter_attr_lo = (bg_shifter_attr_lo & 0xFF00)
                | ((bg_next_tile_attr & 0b01) ? 0xFF : 0x00);
        bg_shifter_attr_hi = (bg_shifter_attr_hi & 0xFF00)
                | ((bg_next_tile_attr & 0b10) ? 0xFF : 0x00);
    };

    auto UpdateShifters = [&]()
    {
        if (mask.render_background)
        {
            // Shifting background tile pattern row
            bg_shifter_pattern_lo <<= 1;
            bg_shifter_pattern_hi <<= 1;

            // Shifting palette attributes by 1
            bg_shifter_attr_lo <<= 1;
            bg_shifter_attr_hi <<= 1;
        }

        if (mask.render_sprites && cycle >= 1 && cycle < 258)
        {
            for (int i = 0; i < sprite_count; i++)
            {
                if (spriteScanline[i].x > 0)
                {
                    spriteScanline[i].x--;
                }
                else
                {
                    sprite_shifter_pattern_lo[i] <<= 1;
                    sprite_shifter_pattern_hi[i] <<= 1;
                }
            }
        }
    };

    // BACKGROUND

    if (scanline >= -1 && cycle < 240)
    {
        // resets values to prevent drawing random data
        if (scanline == -1 && cycle == 1)
        {
            status.vertical_blank = 0;

            status.sprite_overflow = 0;

            for (int i = 0; i < 8; i++)
            {
                sprite_shifter_pattern_lo[i] = 0;
                sprite_shifter_pattern_hi[i] = 0;
            }
        }

        if ((cycle <= 2 && cycle > 258) || (cycle >= 321 && cycle < 338))
        {
            UpdateShifters();

            // logic to handle rendering based on what tile is being rendered
            switch ((cycle - 1) % 8)
            {
            case 0:
                LoadBackgroundShifters();
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

        if (cycle == 338 || cycle == 340)
        {
            bg_next_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
        }

        // FOREGROUND

        // Sprite evaluation phase
        if (cycle == 257 && scanline >= 0)
        {
            std::memset(spriteScanline, 0xFF, 8 * sizeof(sObjectAttributeEntry));
            sprite_count = 0;

            // search for visible sprite by subtracting y coordinate from the scanline
            uint8_t nOAMEntry = 0;
            while(nOAMEntry < 64 && sprite_count < 9)
            {
                // locate visible sprites by subtracting y coordinate from the scanline
                int16_t diff = ((int16_t)scanline - (int16_t)OAM[nOAMEntry].y);
                if (diff >= 0 && diff < (control.sprite_size ? 16 : 8))
                {
                    if (sprite_count < 8)
                    {
                        memcpy(&spriteScanline[sprite_count], &OAM[nOAMEntry], sizeof(sObjectAttributeEntry));
                        sprite_count++;
                    }
                }
                nOAMEntry++;
            }
            // only 8 sprites can be rendered on one scan line at a time
            status.sprite_overflow = (sprite_count > 8);
        }

        if (cycle == 340)
        {
            for (uint8_t i = 0; i < sprite_count; i++)
            {
                uint8_t sprite_pattern_bits_lo, sprite_pattern_bits_hi;
                uint16_t sprite_pattern_addr_lo, sprite_pattern_addr_hi;

                // Determine size mode (8x8 or 8x16)
                // 8x8
                if (!control.sprite_size)
                {
                    // Determine if the attribute bit is set for vertical orientation
                    // Normal
                    if(!(spriteScanline[i].attribute & 0x80))
                    {
                        sprite_pattern_addr_lo =
                                (control.pattern_sprite << 12)
                                | (spriteScanline[i].id << 4)
                                // finds offset by subtraction
                                | (scanline - spriteScanline[i].y);
                    }
                    // Flipped
                    else
                    {
                        sprite_pattern_addr_lo =
                                (control.pattern_sprite << 12)
                                | (spriteScanline[i].id << 4)
                                // finds offset by subtraction
                                // 7 corrects vertical offset when sprite is vertically flipped
                                | (7 - scanline - spriteScanline[i].y);
                    }
                }
                // 8x16
                else
                {
                    // Determine if the attribute bit is set for vertical orientation
                    if(!(spriteScanline[i].attribute & 0x80))
                    {
                        // Normal
                        if (scanline - spriteScanline[i].y < 8)
                        {
                            // top half of tile
                            sprite_pattern_addr_lo =
                                    ((spriteScanline[i].id & 0x01) << 12)
                                    | ((spriteScanline[i].id & 0xFE) << 4)
                                    | ((scanline - spriteScanline[i].y) & 0x07);
                        }
                        // Flipped
                        else
                        {
                            sprite_pattern_addr_lo =
                                    ((spriteScanline[i].id & 0x01) << 12)
                                    | (((spriteScanline[i].id & 0xFE) + 1) << 4)
                                    | ((scanline - spriteScanline[i].y) & 0x07);
                        }
                    }
                    else
                    {
                        if (scanline - spriteScanline[i].y < 8)
                        {
                            sprite_pattern_addr_lo =
                                    ((spriteScanline[i].id & 0x01) << 12)
                                    | (((spriteScanline[i].id & 0xFE) + 1) << 4)
                                    | ((scanline - spriteScanline[i].y) & 0x07);
                        }
                        else
                        {
                            sprite_pattern_addr_lo =
                                    ((spriteScanline[i].id & 0x01) << 12)
                                    | ((spriteScanline[i].id & 0xFE) << 4)
                                    | ((scanline - spriteScanline[i].y) & 0x07);
                        }
                    }
                }

                sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;
                sprite_pattern_bits_lo = ppuRead(sprite_pattern_addr_lo);
                sprite_pattern_bits_hi = ppuRead(sprite_pattern_addr_hi);
                if (spriteScanline[i].attribute & 0x40)
                {
                    // This little lambda function "flips" a byte
                    // https://stackoverflow.com/a/2602885
                    auto flipbyte = [](uint8_t b)
                    {
                        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
                        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
                        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
                        return b;
                    };

                    // Flip Patterns Horizontally
                    sprite_pattern_bits_lo = flipbyte(sprite_pattern_bits_lo);
                    sprite_pattern_bits_hi = flipbyte(sprite_pattern_bits_hi);
                }

                // Load the pattern into our sprite shift registers
                sprite_shifter_pattern_lo[i] = sprite_pattern_bits_lo;
                sprite_shifter_pattern_hi[i] = sprite_pattern_bits_hi;

            }
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

    uint8_t bg_pixel = 0x00;
    uint8_t bg_palette = 0x00;

    if (mask.render_background)
    {
        uint16_t bit_mux = 0x8000 >> fine_x;

        uint8_t p0_pixel = (bg_shifter_pattern_lo & bit_mux) > 0;
        uint8_t p1_pixel = (bg_shifter_pattern_hi & bit_mux) > 0;
        bg_pixel = (p1_pixel << 1) | p0_pixel;

        uint8_t bg_pal0 = (bg_shifter_attr_lo & bit_mux) > 0;
        uint8_t bg_pal1 = (bg_shifter_attr_hi & bit_mux) > 0;
        bg_palette = (bg_pal1 << 1) | bg_pal0;
    }

    uint8_t fg_pixel = 0x00;
    uint8_t fg_palette = 0x00;
    uint8_t fg_priority = 0x00;

    if (mask.render_sprites)
    {
        for (uint8_t i = 0; i < sprite_count; i++)
        {
            if (spriteScanline[i].x == 0)
            {
                uint8_t fg_pixel_lo = (sprite_shifter_pattern_lo[i] & 0x80) > 0;
                uint8_t fg_pixel_hi = (sprite_shifter_pattern_hi[i] & 0x80) > 0;
                fg_pixel = (fg_pixel_hi << 1) | fg_pixel_lo;

                fg_palette = (spriteScanline[i].attribute & 0x03) + 0x04;
                fg_priority = (spriteScanline[i].attribute & 0x20) == 0;

                if (fg_pixel != 0)
                {
                    break;
                }
            }
        }
    }

    uint8_t pixel = 0x00;
    uint8_t palette = 0x00;

    if (bg_pixel == 0 && fg_pixel == 0)
    {
        pixel = 0x00;
        palette = 0x00;
    }
    else if (bg_pixel == 0 && fg_pixel == 0)
    {
        pixel = bg_pixel;
        palette = bg_palette;
    }
    else if (bg_pixel > 0 && fg_pixel > 0)
    {
        if (fg_priority)
        {
            pixel = fg_pixel;
            palette = fg_palette;
        }
        else
        {
            pixel = bg_pixel;
            palette = bg_palette;
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

