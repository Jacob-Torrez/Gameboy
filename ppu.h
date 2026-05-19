#ifndef PPU_H
#define PPU_H

#include <SDL2/SDL.h>
#include "mmu.h"

typedef enum {
    MODE_HBLANK, MODE_VBLANK, MODE_OAM, MODE_DRAW
} mode_t;

typedef enum {
    COLOR_WHITE, COLOR_LGRAY, COLOR_DGRAY, COLOR_BLACK
} color_t;

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
    uint8_t window_line_counter;

    uint8_t lcdc; // 0xFF40
    uint8_t stat; // 0xFF41
    uint8_t scy; // 0xFF42
    uint8_t scx; // 0xFF43
    uint8_t ly; // 0xFF44
    uint8_t lyc; // 0xFF45
    uint8_t bgp; // 0xFF47
    uint8_t obp0; // 0xFF48
    uint8_t obp1; // 0xFF49
    uint8_t wy; // 0xFF4A
    uint8_t wx; // 0xFF4B

    uint8_t vram[0x2000]; // 0x8000 - 0x9FFF
    uint8_t oam[0xA0]; // 0xFE00 - 0xFE9F

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
void render_scanline(PPU* ppu);

void set_stat_mode(PPU* ppu);
void set_ly(PPU* ppu);

uint8_t get_cycles_needed(uint8_t mode);
uint32_t get_color(color_t color);

#endif