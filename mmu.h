#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdio.h>
#include "ppu.h"
#include "timer.h"
#include "joypad.h"

typedef enum {
    INT_VBLANK, INT_LCD, INT_TIMER, INT_SERIAL, INT_JOYPAD
} interrupt_t;

typedef struct {
    uint8_t* rom;
    uint8_t* bios;

    uint8_t* rom_bank0; // 0x0000 - 0x3FFF
    uint8_t* rom_bankN; // 0x4000 - 0x7FFF

    uint8_t* external_ram; 
    uint8_t* eram_bankN; // 0xA000 - 0xBFFF

    uint8_t wram[0x2000]; // 0xC000 - 0xDFFF

    uint8_t hram[0x7F]; // 0xFF80 - 0xFFFE

    uint8_t bios_enabled; // If read_byte reads from BIOS or ROM

    uint8_t dma; // 0xFF46
    uint8_t dma_cycles; // Cycles left in DMA transfer

    uint8_t bank; // 0xFF50

    uint8_t ifl; // 0xFF0F
    uint8_t ie; // 0xFFFF

    PPU* ppu;
    TIMER* timer;
    JOYPAD* joypad;

} MMU;

uint8_t read_byte(MMU* mmu, uint16_t addr);
void write_byte(MMU* mmu, uint16_t addr, uint8_t val);

void request_interrupt(MMU* mmu, interrupt_t interrupt);

uint8_t mmu_init(MMU* mmu, const char* filename);

#endif