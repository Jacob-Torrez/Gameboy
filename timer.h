#ifndef TIMER_H
#define TIMER_H

#include "mmu.h"

typedef struct {

    uint16_t sys_counter;

    MMU* mmu;

} TIMER;

uint8_t timer_step(TIMER* timer, uint8_t cycles);
void timer_init(TIMER* timer);
void timer_reset_sys_counter(TIMER* timer);

#endif