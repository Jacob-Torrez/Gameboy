#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef struct TIMER {

    uint16_t sys_counter; // internal / 0xFF04 (DIV)
    uint8_t tima; // 0xFF05
    uint8_t tma; // 0xFF06
    uint8_t tac; // 0xFF07

} TIMER;

uint8_t timer_step(TIMER* timer, uint8_t cycles);

uint8_t timer_init(TIMER* timer);

#endif