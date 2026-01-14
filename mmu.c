#include "mmu.h"

uint8_t read_byte(MMU* mmu, uint16_t addr){
    return mmu->memory[addr];
}

void write_byte(MMU* mmu, uint16_t addr, uint8_t val){
    if (addr >= 0x0000 && addr <= 0x7FFF){ // ROM
        return;
    }
    else if (addr >= 0xFEA0 && addr <= 0xFEFF){ // Not Usable (TODO: OAM)
        return;
    }
    else if (addr >= 0xC000 && addr <= 0xDDFF){ // WRAM -> Echo RAM
        mmu->memory[addr + 0x2000] = val;
    }
    else if (addr >= 0xE000 && addr <= 0xFDFF){ // Echo RAM -> WRAM
        mmu->memory[addr - 0x2000] = val;
    }
    mmu->memory[addr] = val;
}

void read_ROM(MMU* mmu, const char* filename){
    FILE* file = fopen(filename, "rb");

    if (file == NULL){
        printf("Error: Could not open ROM file %s\n", filename);
        return;
    }

    fread(mmu->memory, sizeof(uint8_t), 0x8000, file);

    fclose(file);
}