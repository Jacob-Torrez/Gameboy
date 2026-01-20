#ifndef PPU_H
#define PPU_H

#include <SDL2/SDL.h>
#include "mmu.h"

typedef enum {
    MODE_HBLANK, MODE_VBLANK, MODE_OAM, MODE_DRAW
} mode_t;

typedef struct{
    uint8_t y;
    uint8_t x;
    uint8_t tile_index;
    uint8_t attributes;
} SpriteAttributes;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    mode_t mode; // 0: HBlank, 1: VBlank, 2: OAM, 3: Draw
    uint8_t mode_cycles;
    uint8_t LY;
    uint8_t window_line_counter;

    uint32_t frame_buffer[160 * 144];
    SpriteAttributes sprite_buffer[10];
    uint8_t sprite_count;

    uint8_t enabled;

    MMU* mmu;
} PPU;

void ppu_step(PPU* ppu, uint8_t cycles);
void ppu_reset(PPU* ppu);

void ppu_init(PPU* ppu);
void ppu_destroy(PPU* ppu);

void scan_oam(PPU* ppu);
void render_scanline(PPU* ppu, uint8_t LY);

void set_stat_mode(PPU* ppu);
void set_ly(PPU* ppu);

uint8_t get_cycles_needed(uint8_t mode);

#endif