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

    union
    {
        struct
        {
            uint8_t unused : 5;
            uint8_t sprite_overflow : 1;
            uint8_t sprite_zero_hit : 1;
            uint8_t vertical_blank : 1;
        };

        uint8_t reg;
    } status;

    union
    {
        struct
        {
            uint8_t grayscale : 1;
            uint8_t render_background_left : 1;
            uint8_t render_sprites_left : 1;
            uint8_t render_background : 1;
            uint8_t render_sprites : 1;
            uint8_t enhance_red : 1;
            uint8_t enhance_green : 1;
            uint8_t enhance_blue : 1;
        };

        uint8_t reg;
    }  mask;

    union PPUCTRL
    {
        struct
        {
            uint8_t nametable_x : 1;
            uint8_t nametable_y : 1;
            uint8_t increment_mode : 1;
            uint8_t pattern_sprite : 1;
            uint8_t pattern_background : 1;
            uint8_t sprite_size : 1;
            uint8_t slave_mode : 1;
            uint8_t enable_nmi : 1;
        };

        uint8_t reg;
    } control;

    uint8_t address_latch = 0x00;
    uint8_t ppu_data_buffer = 0x00;

    union loopy_register
    {
        struct
        {
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nametable_x : 1;
            uint16_t nametable_y : 1;
            uint16_t fine_y : 3;
            uint16_t unused : 1;
        };

        uint16_t reg = 0x0000;
    };

    loopy_register vram_addr;
    loopy_register tram_addr;

    uint8_t fine_x = 0x00;

    uint8_t bg_next_tile_id = 0x00;
    uint8_t bg_next_tile_attr = 0x00;
    uint8_t bg_next_tile_lsb = 0x00;
    uint8_t bg_next_tile_msb = 0x00;

    uint16_t bg_shifter_pattern_lo = 0x0000;
    uint16_t bg_shifter_pattern_hi = 0x0000;
    uint16_t bg_shifter_attr_lo = 0x0000;
    uint16_t bg_shifter_attr_hi = 0x0000;

public:
    // Interface
    void ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge);
    void clock();

    bool nmi = false;
};
#endif // OLC2C02_H
