#include "ppu.h"

void ppu_step(PPU* ppu, uint8_t cycles){
    uint8_t LCD_en = read_byte(ppu->mmu, 0xFF40) >> 7; // 0: off 1: on (PPU/LCD)
    if (LCD_en == 0){
        if (ppu->enabled == 1){
            ppu_reset(ppu);
            ppu->enabled = 0;
        }
        return;
    }
    ppu->enabled = 1;

    ppu->mode_cycles += cycles;

    uint8_t cycles_needed = get_cycles_needed(ppu->mode);

    if (ppu->mode_cycles >= cycles_needed){
        ppu->mode_cycles -= cycles_needed;

        switch (ppu->mode){
            case MODE_HBLANK:
                ppu->LY++;
                set_ly(ppu);

                if (ppu->LY == 144){
                    ppu->mode = MODE_VBLANK;
                    request_interrupt(ppu->mmu, INT_VBLANK);
                } else {
                    ppu->mode = MODE_OAM;
                }
                set_stat_mode(ppu);

                break;
            case MODE_VBLANK:
                ppu->LY = (ppu->LY + 1) % 154;
                set_ly(ppu);

                if (ppu->LY == 0){
                    ppu->window_line_counter = 0;
                    ppu->mode = MODE_OAM;
                    set_stat_mode(ppu);
                }

                break;
            case MODE_OAM:
                scan_oam(ppu);

                ppu->mode = MODE_DRAW;
                set_stat_mode(ppu);

                break;
            case MODE_DRAW:
                render_scaline(ppu, ppu->LY);

                ppu->mode = MODE_HBLANK;
                set_stat_mode(ppu);

                break;
        }
    }
}

void ppu_init(PPU* ppu){
    SDL_Init(SDL_INIT_VIDEO);

    ppu->window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160, 144, 0);
    ppu->renderer = SDL_CreateRenderer(ppu->window, -1, 0);
    ppu->texture = SDL_CreateTexture(ppu->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    ppu->mode = 2;
    ppu->mode_cycles = 0;
    ppu->LY = 0;
    ppu->enabled = 1;
    ppu->window_line_counter = 0;
}

void ppu_destroy(PPU* ppu){
    SDL_DestroyTexture(ppu->texture);
    ppu->texture = NULL;
    SDL_DestroyRenderer(ppu->renderer);
    ppu->renderer = NULL;
    SDL_DestroyWindow(ppu->window);
    ppu->window = NULL;
}

void scan_oam(PPU* ppu){
    ppu->sprite_count = 0;

    uint8_t sprite_height = ((read_byte(ppu->mmu, 0xFF40) & (1 << 2)) >> 2) ? 16 : 8; // LCDC bit 2
    uint8_t LY = ppu->LY;

    for (int i = 0; i < 160; i+=4){
        if (ppu->sprite_count < 10){
            uint8_t y = read_byte(ppu->mmu, 0xFE00 + i);
            if ((int16_t)LY >= (int16_t)y - 16 && (int16_t)LY < (int16_t)y - 16 + sprite_height){
                uint8_t x = read_byte(ppu->mmu, 0xFE00 + i + 1);
                uint8_t tile_index = read_byte(ppu->mmu, 0xFE00 + i + 2);
                uint8_t attributes = read_byte(ppu->mmu, 0xFE00 + i + 3);
                
                ppu->sprite_buffer[ppu->sprite_count] = (SpriteAttributes){y, x, tile_index, attributes};
                ppu->sprite_count++;
            }
        }
    }
}

void render_scanline(PPU* ppu, uint8_t LY){
    uint8_t SCY = read_byte(ppu->mmu, 0xFF42); // Background viewport Y
    uint8_t SCX = read_byte(ppu->mmu, 0xFF43); // Background viewport X
    uint8_t WY = read_byte(ppu->mmu, 0xFF4A); // Window Y
    uint8_t WX = read_byte(ppu->mmu, 0xFF4B); // Window X + 7
            WX = (WX < 7) ? 7 : WX;
    uint8_t LCDC = read_byte(ppu->mmu, 0xFF40); // LCD control

    uint8_t bg_tile_map = (LCDC & 0x08) >> 3; // 1: 9C00-9FFF
    uint8_t w_tile_map = (LCDC & 0x40) >> 6;  // 0: 9800-9BFF
    
    uint8_t tile_data = (LCDC & 0x10) >> 4; // 0: 8800-97FF 1: 8000-8FFF

    uint8_t obj_en = (LCDC & 0x02) >> 1; // 1: on 0: off
    uint8_t obj_size = (LCDC & 0x04) >> 2; // 0: 8x8 1: 8x16 

    uint8_t bg_en = (LCDC & 0x01); // 1: on 0: off
    uint8_t w_en = (LCDC & 0x01) & ((LCDC & 0x20) >> 5); // 1: on 0: off

    uint8_t window_used = 0;
    for (int i = 0; i < 160; i++){
        uint32_t w_pixel;
        uint32_t bg_pixel;
        uint32_t obj_pixel;
        if (w_en == 1 && WY <= LY && (WX - 7) <= i){
            window_used = 1;
            uint16_t tile_map_addr = 0x9800 | (w_tile_map << 10) | ((ppu->window_line_counter >> 3) << 5) | ((i - (WX - 7)) >> 3);
            uint8_t tile_id = read_byte(ppu->mmu, tile_map_addr);
            uint16_t tile_addr;
            if (tile_data == 1){
                tile_addr = 0x8000 + ((uint16_t)tile_id << 4);
            } else {
                tile_addr = 0x9000 + ((int8_t)tile_id << 4);
            }
            tile_addr = tile_addr + (LY % 8);
            uint8_t lo = read_byte(ppu->mmu, tile_addr);
            uint8_t hi = read_byte(ppu->mmu, tile_addr + 1);
            w_pixel = ((hi & (1 << (i % 8))) >> (i % 8)) << 1 | ((lo & (1 << (i % 8))) >> (1 % 8));
        }
        if (bg_en == 1){

        }
        if (obj_en == 1){

        }
    }

    ppu->window_line_counter += window_used;
}

void set_stat_mode(PPU* ppu){
    uint8_t stat = read_byte(ppu->mmu, 0xFF41);
    stat &= ~0x03;
    stat |= ppu->mode;
    write_byte(ppu->mmu, 0xFF41, stat);

    if ((ppu->mode == MODE_OAM && (stat & (1 << 5)) != 0) || 
        (ppu->mode == MODE_VBLANK && (stat & (1 << 4)) != 0) ||
        (ppu->mode == MODE_HBLANK && (stat & (1 << 3)) != 0)){
            request_interrupt(ppu->mmu, INT_LCD);
        }
}

void set_ly(PPU* ppu){
    uint8_t LY = ppu->LY;
    write_byte(ppu->mmu, 0xFF44, LY);

    uint8_t LYC = read_byte(ppu->mmu, 0xFF45);
    uint8_t stat = read_byte(ppu->mmu, 0xFF41);

    stat &= ~(1 << 2);
    stat |= ((LYC == LY) ? 1 : 0) << 2;

    write_byte(ppu->mmu, 0xFF41, stat);

    if (LYC == LY && (stat & (1 << 6)) != 0){
        request_interrupt(ppu->mmu, INT_LCD);
    }
}

uint8_t get_cycles_needed(uint8_t mode){
    switch(mode){
        case MODE_HBLANK: return 204;
        case MODE_VBLANK: return 456;
        case MODE_OAM: return 80;
        case MODE_DRAW: return 172;
        default: return 0;
    }
}