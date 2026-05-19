#include "mmu.h"

uint8_t read_byte(MMU* mmu, uint16_t addr){
    if (addr >= 0x0000 && addr <= 0x3FFF){ // ROM Bank 00
        return mmu->rom_bank0[addr];
    }
    if (addr >= 0x4000 && addr <= 0x7FFF){ // ROM Bank 01-NN (TODO MBC)
        return mmu->rom_bankN[addr - 0x4000];
    }
    if (addr >= 0x8000 && addr <= 0x9FFF){ // VRAM
        if (mmu->ppu->mode != MODE_DRAW) {
            return mmu->ppu->vram[addr - 0x8000];
        } else {
            return 0xFF;
        }
    }
    if (addr >= 0xA000 && addr <= 0xBFFF){ // External RAM (TODO SWAP?)
        return mmu->eram_bankN[addr - 0xA000];
    }
    if (addr >= 0xC000 && addr <= 0xDFFF){ // Work RAM
        return mmu->wram[addr - 0xC000];
    }
    if (addr >= 0xE000 && addr <= 0xFDFF){ // Echo RAM
        return mmu->wram[addr - 0xE000];
    }
    if (addr >= 0xFE00 && addr <= 0xFE9F){ // OAM
        if (mmu->ppu->mode < 2) {
            return mmu->ppu->oam[addr - 0xFE00];
        } else {
            return 0xFF;
        }
    }
    if (addr >= 0xFEA0 && addr <= 0xFE9F){ // Not Usable (TODO?)
        return 0xFF;
    }
    if (addr >= 0xFF00 && addr <= 0xFF7F){ // I/O Registers
        switch (addr) {
            case 0xFF00: // JOYP
                if ((mmu->joypad->action_direction[0] & 0x20) == 0){
                    return mmu->joypad->action_direction[1] & 0xF;
                }
                else if ((mmu->joypad->action_direction[0] & 0x10) == 0){
                    return mmu->joypad->action_direction[0] & 0xF;
                } else {
                    return 0xF;
                }

            case 0xFF04: return mmu->timer->sys_counter >> 8; // DIV
            case 0xFF05: return mmu->timer->tima; // TIMA
            case 0xFF06: return mmu->timer->tma; // TMA
            case 0xFF07: return mmu->timer->tac; // TAC
            case 0xFF0F: return mmu->ifl | 0xE0; // IF
            case 0xFF40: return mmu->ppu->lcdc; // LCDC
            case 0xFF41: return mmu->ppu->stat; // STAT
            case 0xFF42: return mmu->ppu->scy; // SCY
            case 0xFF43: return mmu->ppu->scx; // SCX
            case 0xFF44: return mmu->ppu->ly; // LY
            case 0xFF45: return mmu->ppu->lyc; // LYC
            case 0xFF47: return mmu->ppu->bgp; // BGP
            case 0xFF48: return mmu->ppu->obp0; // OBP0
            case 0xFF49: return mmu->ppu->obp1; // OBP1
            case 0xFF4A: return mmu->ppu->wy; // WY
            case 0xFF4B: return mmu->ppu->wx; // WX

            case 0xFF46: // DMA (TODO)
                return 0xFF; 

            default:
                return 0xFF;
        }
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE){ // HRAM
        return mmu->hram[addr - 0xFF80];
    }
    if (addr == 0xFFFF){
        return mmu->ie;
    }

    return 0xFF;
}

void write_byte(MMU* mmu, uint16_t addr, uint8_t val){ // TODO
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

void request_interrupt(MMU* mmu, interrupt_t interrupt){
    uint8_t ifl = mmu->ifl;
    ifl |= 1 << interrupt;
    mmu->ifl = ifl;
}

void read_ROM(MMU* mmu, const char* filename){ // TODO
    FILE* file = fopen(filename, "rb");

    if (file == NULL){
        printf("Error: Could not open ROM file %s\n", filename);
        return;
    }

    fread(mmu->memory, sizeof(uint8_t), 0x8000, file);

    fclose(file);
}