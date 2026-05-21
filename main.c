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

            if (cpu->set_ime_next){
                cpu->ime = 1;
                cpu->set_ime_next = 0;
            }

            ppu_step(cpu->mmu->ppu, cycles);

            uint8_t timer_overflow = timer_step(cpu->mmu->timer, cycles);
            if (timer_overflow){
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
    const char* rom_path = (argc > 1) ? argv[1] : "Tetris.gb";

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

    if (mmu.rom) free(mmu.rom);
    if (mmu.bios) free(mmu.bios);
    
    if (ppu.texture) SDL_DestroyTexture(ppu.texture);
    if (ppu.renderer) SDL_DestroyRenderer(ppu.renderer);
    if (ppu.window) SDL_DestroyWindow(ppu.window);
    
    SDL_Quit();

    return 0;
}