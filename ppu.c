#include "ppu.h"
#include "mmu.h"

#define SCALE 3

static void ppu_reset(PPU* ppu);
static void scan_oam(PPU* ppu);
static void render_scanline(PPU* ppu);
static void set_stat_mode(PPU* ppu);
static void set_ly(PPU* ppu);
static uint16_t get_cycles_needed(ppu_mode_t mode);
static uint32_t get_color(color_t color);

void ppu_step(PPU* ppu, uint8_t cycles){
    uint8_t LCD_en = ppu->lcdc >> 7; // 0: off 1: on (PPU/LCD)
    if (LCD_en == 0){
        if (ppu->enabled == 1){
            ppu_reset(ppu);
            ppu->enabled = 0;
        }
        return;
    }
    ppu->enabled = 1;

    ppu->mode_cycles += cycles;

    uint16_t cycles_needed = get_cycles_needed(ppu->mode);

    if (ppu->mode_cycles >= cycles_needed){
        ppu->mode_cycles -= cycles_needed;

        switch (ppu->mode){
            case MODE_HBLANK:
                ppu->ly++;
                set_ly(ppu);

                if (ppu->ly == 144){
                    ppu->mode = MODE_VBLANK;
                    request_interrupt(ppu->mmu, INT_VBLANK);
                } else {
                    ppu->mode = MODE_OAM;
                }
                set_stat_mode(ppu);

                break;
            case MODE_VBLANK:
                ppu->ly = (ppu->ly + 1) % 154;
                set_ly(ppu);

                if (ppu->ly == 0){
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
                render_scanline(ppu);

                ppu->mode = MODE_HBLANK;
                set_stat_mode(ppu);

                break;
        }
    }
}

uint8_t ppu_init(PPU* ppu){
    SDL_Init(SDL_INIT_VIDEO);

    ppu->window = SDL_CreateWindow("Gameboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * SCALE, 144 * SCALE, 0);
    ppu->renderer = SDL_CreateRenderer(ppu->window, -1, 0);
    SDL_RenderSetLogicalSize(ppu->renderer, 160, 144);
    SDL_RenderSetIntegerScale(ppu->renderer, SDL_TRUE);
    ppu->texture = SDL_CreateTexture(ppu->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    ppu->mode = 2;
    ppu->mode_cycles = 0;
    ppu->enabled = 1;
    ppu->window_line_counter = 0;
    ppu->sprite_count = 0;

    ppu->lcdc = 0;
    ppu->stat = 0;
    ppu->scy = 0;
    ppu->scx = 0;
    ppu->ly = 0;
    ppu->lyc = 0;
    ppu->obp0 = 0;
    ppu->obp1 = 0;
    ppu->wy = 0;
    ppu->wx = 0;

    return 0;
}

void ppu_destroy(PPU* ppu){
    SDL_DestroyTexture(ppu->texture);
    ppu->texture = NULL;
    SDL_DestroyRenderer(ppu->renderer);
    ppu->renderer = NULL;
    SDL_DestroyWindow(ppu->window);
    ppu->window = NULL;
}

static void ppu_reset(PPU* ppu){
    ppu->ly = 0;
    ppu->mode_cycles = 0;

    ppu->mode = MODE_OAM;

    memset(ppu->frame_buffer, 0, sizeof(ppu->frame_buffer));
}

static void scan_oam(PPU* ppu){
    ppu->sprite_count = 0;

    uint8_t sprite_height = ((ppu->lcdc & (1 << 2)) >> 2) ? 16 : 8; // lcdc bit 2
    uint8_t ly = ppu->ly;

    for (int i = 0; i < 160; i+=4){
        if (ppu->sprite_count < 10){
            uint8_t y = ppu->oam[i];
            if ((int16_t)ly >= (int16_t)y - 16 && (int16_t)ly < (int16_t)y - 16 + sprite_height){
                uint8_t x = ppu->oam[i + 1];
                uint8_t tile_index = ppu->oam[i + 2];
                uint8_t attributes = ppu->oam[i + 3];
                
                ppu->sprite_buffer[ppu->sprite_count] = (SpriteAttributes){y, x, tile_index, attributes};
                ppu->sprite_count++;
            }
        }
    }
}

static void render_scanline(PPU* ppu){
    uint8_t scy = ppu->scy; // Background viewport Y
    uint8_t scx = ppu->scx; // Background viewport X
    uint8_t wy = ppu->wy; // Window Y
    uint8_t wx = ppu->wx; // Window X + 7
            wx = (wx < 7) ? 7 : wx;

    uint8_t bgp = ppu->bgp; // Background Palette
    uint8_t obp0 = ppu->obp0; // Object Palette 0
    uint8_t obp1 = ppu->obp1; // Object Pallete 1

    uint8_t bg_tile_map = (ppu->lcdc & 0x08) >> 3; // 1: 9C00-9FFF
    uint8_t w_tile_map = (ppu->lcdc & 0x40) >> 6;  // 0: 9800-9BFF
    
    uint8_t tile_data = (ppu->lcdc & 0x10) >> 4; // 0: 8800-97FF 1: 8000-8FFF

    uint8_t obj_en = (ppu->lcdc & 0x02) >> 1; // 1: on 0: off
    uint8_t obj_size = (ppu->lcdc & 0x04) >> 2; // 0: 8x8 1: 8x16

    uint8_t bg_en = (ppu->lcdc & 0x01); // 1: on 0: off
    uint8_t w_en = (ppu->lcdc & 0x01) & ((ppu->lcdc & 0x20) >> 5); // 1: on 0: off

    uint8_t window_used = 0;
    for (int i = 0; i < 160; i++){
        uint32_t bgw_pixel = 0;
        uint32_t obj_pixel = 0;
        if (w_en == 1 && wy <= ppu->ly && (wx - 7) <= i){
            window_used = 1;
            uint16_t tile_map_addr = 0x9800 | (w_tile_map << 10) | ((ppu->window_line_counter >> 3) << 5) | ((i - (wx - 7)) >> 3); // 0b10011(tilemap)(Y)(X)
            uint8_t tile_id = ppu->vram[tile_map_addr - 0x8000];
            uint16_t tile_addr;
            if (tile_data == 1){
                tile_addr = 0x8000 + ((uint16_t)tile_id << 4);
            } else {
                tile_addr = 0x9000 + ((int8_t)tile_id << 4);
            }
            tile_addr = tile_addr + ((ppu->window_line_counter % 8) * 2);
            uint8_t lo = ppu->vram[tile_addr - 0x8000];
            uint8_t hi = ppu->vram[tile_addr + 1 - 0x8000];
            uint8_t x_bit_pos = 7 - ((i - (wx - 7)) % 8);
            uint8_t bit_lo = (lo >> x_bit_pos) & 0x01;
            uint8_t bit_hi = (hi >> x_bit_pos) & 0x01;
            bgw_pixel = (bit_hi << 1) | bit_lo;
            ppu->frame_buffer[ppu->ly * 160 + i] = get_color((bgp >> (bgw_pixel * 2)) & 0x3);
        }
        else if (bg_en == 1){ 
            uint16_t tile_map_addr = 0x9800 | (bg_tile_map << 10) | ((((ppu->ly + scy) % 256) >> 3) << 5) | (((i + scx) % 256) >> 3); // 0b10011(tilemap)(Y)(X)
            uint8_t tile_id = ppu->vram[tile_map_addr - 0x8000];
            uint16_t tile_addr;
            if (tile_data == 1){
                tile_addr = 0x8000 + ((uint16_t)tile_id << 4);
            } else {
                tile_addr = 0x9000 + ((int8_t)tile_id << 4);
            }
            tile_addr = tile_addr + (((ppu->ly + scy) % 8) * 2);
            uint8_t lo = ppu->vram[tile_addr - 0x8000];
            uint8_t hi = ppu->vram[tile_addr + 1 - 0x8000];
            uint8_t x_bit_pos = 7 - ((i + scx) % 8);
            uint8_t bit_lo = (lo >> x_bit_pos) & 0x01;
            uint8_t bit_hi = (hi >> x_bit_pos) & 0x01;
            bgw_pixel = (bit_hi << 1) | bit_lo;
            ppu->frame_buffer[ppu->ly * 160 + i] = get_color((bgp >> (bgw_pixel * 2)) & 0x3);
        }
        if (obj_en == 1){
            SpriteAttributes* obj = NULL;
            for (int j = 0; j < ppu->sprite_count; j++){
                if (i >= (ppu->sprite_buffer[j].x - 8) && i < (ppu->sprite_buffer[j].x - 8) + 8){
                    if (!obj || ppu->sprite_buffer[j].x < obj->x){
                        obj = &ppu->sprite_buffer[j];
                    }
                }
            }

            if (obj != NULL){
                uint8_t tile_id;
                uint8_t tile_height; // used to normalize 8x16 tiles into 8x8 standards once tile is selected
                if (obj_size == 0){ // 8x8
                    tile_id = obj->tile_index;
                    tile_height = obj->y;
                } 
                else if ((obj->attributes & 0x40) != 0){ // Y flip 8x16
                    tile_id = (ppu->ly >= (obj->y - 8)) ? obj->tile_index & 0xFE : obj->tile_index | 0x01;
                    tile_height = obj->y + ((tile_id == (obj->tile_index | 0x01)) ? 0 : 8);
                } else { // 8x16
                    tile_id = (ppu->ly >= (obj->y - 8)) ? obj->tile_index | 0x01 : obj->tile_index & 0xFE;
                    tile_height = obj->y + ((tile_id == (obj->tile_index | 0x01)) ? 8 : 0);
                }

                uint16_t tile_addr = 0x8000 + ((uint16_t)tile_id << 4);
                tile_addr = tile_addr + ((((obj->attributes & 0x40) != 0) ? (7 - ppu->ly - (tile_height - 16)) : (ppu->ly - (tile_height - 16))) * 2);
                uint8_t lo = ppu->vram[tile_addr - 0x8000];
                uint8_t hi = ppu->vram[tile_addr + 1 - 0x8000];
                uint8_t x_bit_pos = ((obj->attributes & 0x20) != 0) ? ((i - obj->x + 8) % 8) : 7 - ((i - obj->x + 8) % 8);
                uint8_t bit_lo = (lo >> x_bit_pos) & 0x01;
                uint8_t bit_hi = (hi >> x_bit_pos) & 0x01;
                obj_pixel = (bit_hi << 1) | bit_lo;
                if (obj_pixel != 0 && ((bg_en == 0) || (bgw_pixel == 0) || ((obj->attributes & 0x80) == 0))){
                    obj_pixel = ((((obj->attributes & 0x10) != 0) ? obp1 : obp0) >> (obj_pixel * 2)) & 0x3;
                    ppu->frame_buffer[ppu->ly * 160 + i] = get_color(obj_pixel);
                }
            }
        }
    }

    ppu->window_line_counter += window_used;
}

static void set_stat_mode(PPU* ppu){
    uint8_t stat = ppu->stat;
    stat &= ~0x03;
    stat |= ppu->mode;
    ppu->stat = stat;

    if ((ppu->mode == MODE_OAM && (stat & (1 << 5)) != 0) || 
        (ppu->mode == MODE_VBLANK && (stat & (1 << 4)) != 0) ||
        (ppu->mode == MODE_HBLANK && (stat & (1 << 3)) != 0)){
            request_interrupt(ppu->mmu, INT_LCD);
        }
}

static void set_ly(PPU* ppu){
    uint8_t ly = ppu->ly;

    uint8_t lyc = ppu->lyc;
    uint8_t stat = ppu->stat;

    stat &= ~0x4;
    stat |= ((lyc == ly) ? 1 : 0) << 2;

    ppu->stat = stat;

    if (lyc == ly && (stat & (1 << 6)) != 0){
        request_interrupt(ppu->mmu, INT_LCD);
    }
}

static uint16_t get_cycles_needed(ppu_mode_t mode){
    switch(mode){
        case MODE_HBLANK: return 204;
        case MODE_VBLANK: return 456;
        case MODE_OAM: return 80;
        case MODE_DRAW: return 172;
        default: return 0;
    }
}

static uint32_t get_color(color_t color){
    switch(color){
        case COLOR_BLACK: return 0xFF000000;
        case COLOR_WHITE: return 0xFFFFFFFF;
        case COLOR_LGRAY: return 0xFFD3D3D3;
        case COLOR_DGRAY: return 0xFF404040;
    }
}