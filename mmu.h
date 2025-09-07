#ifndef MMU_H
#define MMU_H

#include <stdint.h>

typedef struct {} MMU;

uint8_t read_byte(MMU* mmu, uint16_t addr);
void write_byte(MMU* mmu, uint16_t addr, uint8_t val);

#endif