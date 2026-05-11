#include "timer.h"

uint16_t get_tac_mask(uint8_t mode);

uint8_t timer_step(TIMER* timer, uint8_t cycles){ // TODO: TIMA DELAY
    uint8_t ret_cycles = 0;

    uint16_t mask = get_tac_mask(timer->tac & 0x3);
    uint8_t enabled = (timer->tac & 0x4) != 0;

    while (cycles > 0){
        uint8_t step = (cycles >= 4) ? 4 : cycles;
        cycles -= step;

        uint16_t bit_before = timer->sys_counter & mask;

        timer->sys_counter += step;

        uint16_t bit_after = timer->sys_counter & mask;

        if (enabled && (bit_before != 0) && (bit_after == 0)){
            if (timer->tima == 0xFF){
                ret_cycles = 4;
                timer->tima = 0;
            } else {
                timer->tima++;
            }
        }
    }

    return ret_cycles;
}

void timer_init(TIMER* timer){
    timer->sys_counter = 0;
    timer->tima = 0;
    timer->tma = 0;
    timer->tac = 0;
}

void timer_reset_sys_counter(TIMER* timer){
    timer->sys_counter = 0;
}

uint8_t get_div(TIMER* timer){
    return (timer->sys_counter >> 8);
}

uint8_t get_tima(TIMER* timer){
    return timer->tima;
}

uint8_t get_tma(TIMER* timer){
    return timer->tma;
}

uint8_t get_tac(TIMER* timer){
    return timer->tac;
}

void reset_div(TIMER* timer){ // BUG
    timer->sys_counter = 0;
}

void set_tima(TIMER* timer, uint8_t val){
    timer->tima = val;
}

void set_tma(TIMER* timer, uint8_t val){
    timer->tma = val;
}

void set_tac(TIMER* timer, uint8_t val){
    timer->tac = val;
}

uint16_t get_tac_mask(uint8_t mode){
    switch (mode){
        case 0: return 1 << 9;
        case 1: return 1 << 3;
        case 2: return 1 << 5;
        case 3: return 1 << 7;
    }
}