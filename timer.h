#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef struct {

    uint16_t sys_counter; // internal / 0xFF04 (DIV)
    uint8_t tima; // 0xFF05
    uint8_t tma; // 0xFF06
    uint8_t tac; // 0xFF07

} TIMER;

uint8_t timer_step(TIMER* timer, uint8_t cycles);

void timer_init(TIMER* timer);
void timer_reset_sys_counter(TIMER* timer);

uint8_t get_div(TIMER* timer);
uint8_t get_tima(TIMER* timer);
uint8_t get_tma(TIMER* timer);
uint8_t get_tac(TIMER* timer);

void reset_div(TIMER* timer); // TODO: send timer tick
void set_tima(TIMER* timer, uint8_t val);
void set_tma(TIMER* timer, uint8_t val);
void set_tac(TIMER* timer, uint8_t val); // TODO: send timer tick

#endif