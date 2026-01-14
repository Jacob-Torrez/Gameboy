#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t memory[0x10000];
} MMU;

uint8_t read_byte(MMU* mmu, uint16_t addr);
void write_byte(MMU* mmu, uint16_t addr, uint8_t val);

void read_ROM(MMU* mmu, const char* filename);

#endif