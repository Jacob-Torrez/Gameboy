#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"
#include "joypad.h"
#include "timer.h"

void run_emulator(CPU* cpu){
    uint8_t is_running = 1;
    uint32_t cycle_tracker = 0;

    // Gameboy frame takes 70224 T-cycles (456 cycles * 156 scanlines)
    const uint32_t CYCLES_PER_FRAME = 70224;

    while (is_running){

        uint8_t input_status = process_events(cpu->mmu->joypad);

        if (input_status == 2){
            is_running = 0;
            break;
        } else if (input_status == 1){
            request_interrupt(cpu->mmu, INT_JOYPAD);
            cpu->stopped = 0;
        }

        while (cycle_tracker < CYCLES_PER_FRAME){
            uint8_t cycles = 0;

            uint8_t pending_ime = cpu->set_ime_next;
            cpu->set_ime_next = 0;

            uint8_t int_status = interrupt_handler(cpu);
            if (int_status == 1){
                cpu->halt_bug = 1;
            } else if (int_status == 20) {
                cycles += 20;
            }

            if (cpu->halted){
                cycles += 4;
            } else if (cpu->stopped) {
                cycles += 4;
            } else {
                cycles += cpu_step(cpu);
            }

            if (pending_ime){
                cpu->ime = 1;
            }

            ppu_step(cpu->mmu->ppu, cycles);

            uint8_t timer_overflow = timer_step(cpu->mmu->timer, cycles);
            if (timer_overflow){ // Doesn't account for 4 cycle delay before requesting interrupt
                cpu->mmu->timer->tima = cpu->mmu->timer->tma;
                request_interrupt(cpu->mmu, INT_TIMER);
            }

            cycle_tracker += cycles;
        }

        cycle_tracker -= CYCLES_PER_FRAME;

        SDL_UpdateTexture(cpu->mmu->ppu->texture, NULL, cpu->mmu->ppu->frame_buffer, 160 * sizeof(uint32_t));
        SDL_RenderClear(cpu->mmu->ppu->renderer);
        SDL_RenderCopy(cpu->mmu->ppu->renderer, cpu->mmu->ppu->texture, NULL, NULL);
        SDL_RenderPresent(cpu->mmu->ppu->renderer);

        SDL_Delay(16);
    }
}

int main(int argc, char* argv[]){
    const char* rom_path = argv[1];

    MMU mmu = {0};
    CPU cpu = {0};
    PPU ppu = {0};
    TIMER timer = {0};
    JOYPAD joypad = {0};

    cpu.mmu = &mmu;
    mmu.ppu = &ppu;
    mmu.timer = &timer;
    mmu.joypad = &joypad;

    ppu.mmu = &mmu;

    printf("Initializing Joypad...\n");
    joypad_init(&joypad);

    printf("Initializing Timer...\n");
    timer_init(&timer);

    printf("Initializing PPU & SDL...\n");
    ppu_init(&ppu);

    printf("Loading BIOS and ROM: %s\n", rom_path);
    mmu_init(&mmu, rom_path); 

    printf("Initializing CPU...\n");
    cpu_init(&cpu);

    printf("Starting Emulator...\n");
    run_emulator(&cpu);

    printf("Shutting down...\n");

    if (mmu.mbc_enabled && mmu.external_ram){
        FILE* save = fopen("saves/save.sav", "wb");
        if (save){
            fwrite(mmu.external_ram, 1, 0x8000, save);
            fclose(save);
        }
    }

    mmu_destroy(&mmu);
    ppu_destroy(&ppu);
    
    SDL_Quit();

    return 0;
}