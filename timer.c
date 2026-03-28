#include "timer.h"

uint16_t get_tac_mask(uint8_t mode);

void timer_init(TIMER* timer){
    timer->sys_counter = 0;
}

void timer_reset_sys_counter(TIMER* timer){
    timer->sys_counter = 0;
}

uint8_t timer_step(TIMER* timer, uint8_t cycles){
    uint8_t ret_cycles = 0;

    uint8_t tac = read_byte(timer->mmu, 0xFF07);
    uint16_t mask = get_tac_mask(tac & 0x3);
    uint8_t enabled = (tac & 0x4) != 0;

    while (cycles > 0){
        uint8_t step = (cycles >= 4) ? 4 : cycles;
        cycles -= step;

        uint16_t bit_before = timer->sys_counter & mask;

        timer->sys_counter += step;

        uint16_t bit_after = timer->sys_counter & mask;

        if (enabled && (bit_before != 0) && (bit_after == 0)){
            uint8_t tima = read_byte(timer->mmu, 0xFF05);

            if (tima == 0xFF){
                ret_cycles = 4;
                write_byte(timer->mmu, 0xFF05, 0);
            } else {
                write_byte(timer->mmu, 0xFF05, tima + 1);
            }
            
        }
    }

    return ret_cycles;
}

uint16_t get_tac_mask(uint8_t mode){
    switch (mode){
        case 0: return 1 << 9;
        case 1: return 1 << 3;
        case 2: return 1 << 5;
        case 3: return 1 << 7;
    }
}