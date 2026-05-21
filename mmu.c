#include "mmu.h"
#include "ppu.h"
#include "joypad.h"
#include "timer.h"

static uint8_t read_rom(MMU* mmu, const char* filename);
static uint8_t read_bios(MMU* mmu);

uint8_t read_byte(MMU* mmu, uint16_t addr){
    if (mmu->bios_enabled == 1 && addr < 0x0100){
        return mmu->bios[addr];
    }
    if (addr >= 0x0000 && addr <= 0x3FFF){ // ROM Bank 00
        return mmu->rom_bank0[addr];
    }
    if (addr >= 0x4000 && addr <= 0x7FFF){ // ROM Bank 01-NN (TODO MBC)
        return mmu->rom_bankN[addr - 0x4000];
    }
    if (addr >= 0x8000 && addr <= 0x9FFF){ // VRAM
        if (mmu->ppu->mode != MODE_DRAW){
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
        if (mmu->ppu->mode < MODE_OAM){
            return mmu->ppu->oam[addr - 0xFE00];
        } else {
            return 0xFF;
        }
    }
    if (addr >= 0xFEA0 && addr <= 0xFEFF){ // Not Usable (TODO?)
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
            case 0xFF46: return mmu->dma; // DMA
            case 0xFF47: return mmu->ppu->bgp; // BGP
            case 0xFF48: return mmu->ppu->obp0; // OBP0
            case 0xFF49: return mmu->ppu->obp1; // OBP1
            case 0xFF4A: return mmu->ppu->wy; // WY
            case 0xFF4B: return mmu->ppu->wx; // WX

            default: return 0xFF;
        }
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE){ // HRAM
        return mmu->hram[addr - 0xFF80];
    }
    if (addr == 0xFFFF){ // IE
        return mmu->ie;
    }

    return 0xFF;
}

void write_byte(MMU* mmu, uint16_t addr, uint8_t val){ // TODO
    if (addr >= 0x0000 && addr <= 0x7FFF){ // ROM (TODO: MBC)
        return;
    }
    if (addr >= 0x8000 && addr <= 0x9FFF){ // VRAM
        if (mmu->ppu->mode != MODE_DRAW){
            mmu->ppu->vram[addr - 0x8000] = val;
        }
    }
    if (addr >= 0xA000 && addr <= 0xBFFF){ // External RAM
        mmu->eram_bankN[addr - 0xA000] = val;
    }
    if (addr >= 0xC000 && addr <= 0xDFFF){ // WRAM
        mmu->wram[addr - 0xC000] = val;
    }
    if (addr >= 0xE000 && addr <= 0xFDFF){ // Echo RAM
        mmu->wram[addr - 0xE000] = val;
    }
    if (addr >= 0xFE00 && addr <= 0xFE9F){ // OAM
        if (mmu->ppu->mode < MODE_OAM){
            mmu->ppu->oam[addr - 0xFE00] = val;
        }
    }
    if (addr >= 0xFF00 && addr <= 0xFF7F){ // I/O Registers
        switch (addr) {
            case 0xFF00: // JOYP
                mmu->joypad->action_direction[0] = (mmu->joypad->action_direction[0] & 0x0F) | (val & 0xF0);
                mmu->joypad->action_direction[1] = (mmu->joypad->action_direction[1] & 0x0F) | (val & 0xF0);
                break;

            case 0xFF04: mmu->timer->sys_counter = 0; break; // DIV
            case 0xFF05: mmu->timer->tima = val; break; // TIMA
            case 0xFF06: mmu->timer->tma = val; break; // TMA
            case 0xFF07: mmu->timer->tac = val; break; // TAC
            case 0xFF0F: mmu->ifl = val; break; // IF
            case 0xFF40: mmu->ppu->lcdc = val; break; // LCDC
            case 0xFF41: mmu->ppu->stat = (mmu->ppu->stat & 0x7) | (val & 0xF8); break; // STAT
            case 0xFF42: mmu->ppu->scy = val; break; // SCY
            case 0xFF43: mmu->ppu->scx = val; break; // SCX
            case 0xFF45: mmu->ppu->lyc = val; break; // LYC
            case 0xFF47: mmu->ppu->bgp = val; break; // BGP
            case 0xFF48: mmu->ppu->obp0 = val; break; // OBP0
            case 0xFF49: mmu->ppu->obp1 = val; break; // OBP1
            case 0xFF4A: mmu->ppu->wy = val; break; // WY
            case 0xFF4B: mmu->ppu->wx = val; break; // WX

            case 0xFF50: // BANK
                mmu->bank = val;
                mmu->bios_enabled = 0;
                break;
            
            case 0xFF46:
                mmu->dma = (val > 0xDF) ? val - 0x20 : val;
                mmu->dma_cycles = 640; // 640 T-cycles / 160 M-cycles
                uint8_t* src = mmu->rom_bank0;
                uint16_t src_addr = mmu->dma << 8;
                if (src_addr >= 0x0000 && src_addr <= 0x3F00){src = mmu->rom_bank0 + src_addr;}
                else if (src_addr >= 0x4000 && src_addr <= 0x7F00){src = mmu->rom_bankN + src_addr - 0x4000;}
                else if (src_addr >= 0x8000 && src_addr <= 0x9F00){src = mmu->ppu->vram + src_addr - 0x8000;}
                else if (src_addr >= 0xA000 && src_addr <= 0xBF00){src = mmu->eram_bankN + src_addr - 0xA000;}
                else if (src_addr >= 0xC000 && src_addr <= 0xDF00){src = mmu->wram + src_addr - 0xC000;}
                
                memcpy(mmu->ppu->oam, src, 0xA0); // Potential boundary problem
                break;
            
            default: return;
        }
    }
    if (addr >= 0xFF80 && addr <= 0xFFFE){ // HRAM
        mmu->hram[addr - 0xFF80] = val;
    }
    if (addr == 0xFFFF){ // IE
        mmu->ie = val;
    }
}

void request_interrupt(MMU* mmu, interrupt_t interrupt){
    uint8_t ifl = mmu->ifl;
    ifl |= 1 << interrupt;
    mmu->ifl = ifl;
}

uint8_t mmu_init(MMU* mmu, const char* filename){
    if (read_rom(mmu, filename) != 0){
        return 1;
    }

    if (read_bios(mmu) != 0){
        return 1;
    }

    mmu->rom_bank0 = mmu->rom;
    mmu->rom_bankN = mmu->rom + 0x4000;

    // TODO EXTERNAL RAM?

    mmu->bios_enabled = 1;
    mmu->dma = 0;
    mmu->dma_cycles = 0;
    mmu->bank = 0;
    mmu->ifl = 0;
    mmu->ie = 0;

    return 0;
}

static uint8_t read_rom(MMU* mmu, const char* filename){ // TODO
    FILE* file = fopen(filename, "rb");

    if (file == NULL){
        printf("Error: Could not open ROM file %s\n", filename);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    mmu->rom = calloc(size, sizeof(uint8_t));
    if (mmu->rom == NULL){
        return 1;
    }

    fread(mmu->rom, sizeof(uint8_t), size, file);
    fclose(file);
    return 0;
}

static uint8_t read_bios(MMU* mmu){
    FILE* file = fopen("dmg_boot.gb", "rb");

    if (file == NULL){
        printf("Error: Could not open BIOS file\n");
        return 1;
    }

    mmu->bios = calloc(0x100, sizeof(uint8_t));
    if (mmu->bios == NULL){
        return 1;
    }

    fread(mmu->bios, sizeof(uint8_t), 0x100, file);
    fclose(file);
    return 0;
}