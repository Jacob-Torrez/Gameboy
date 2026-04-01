#ifndef JOYPAD_H
#define JOYPAD_H

#include "mmu.h"
#include <SDL2/SDL.h>

typedef struct {

    uint8_t action_direction[2]; // 0: d-pad, 1: buttons

} JOYPAD;

uint8_t process_events(JOYPAD* joypad);

#endif