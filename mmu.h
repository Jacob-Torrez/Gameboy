#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdio.h>

typedef enum {
    INT_VBLANK, INT_LCD, INT_TIMER, INT_SERIAL, INT_JOYPAD
} interrupt_t;

typedef struct {
    uint8_t memory[0x10000];
} MMU;

uint8_t read_byte(MMU* mmu, uint16_t addr);
void write_byte(MMU* mmu, uint16_t addr, uint8_t val);

void request_interrupt(MMU* mmu, interrupt_t interrupt);

void read_ROM(MMU* mmu, const char* filename);

#endif