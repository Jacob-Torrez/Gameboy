#include "joypad.h"

/* 
0: no new keypress
1: new keypress
2: exit
*/
uint8_t process_events(JOYPAD* joypad){
    uint8_t ret = 0;

    SDL_Event event;

    while (SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                ret = 2;
                break;

            case SDL_KEYDOWN:
                if (event.key.repeat != 0) {break;}
                switch(event.key.keysym.sym){
                    case SDLK_w: // Up
                        joypad->action_direction[0] &= ~0x4;
                        ret = 1;
                        break;
                    
                    case SDLK_a: // Left
                        joypad->action_direction[0] &= ~0x2;
                        ret = 1;
                        break;

                    case SDLK_s: // Down
                        joypad->action_direction[0] &= ~0x8;
                        ret = 1;
                        break;

                    case SDLK_d: // Right
                        joypad->action_direction[0] &= ~0x1;
                        ret = 1;
                        break;

                    case SDLK_e: // B
                        joypad->action_direction[1] &= ~0x2;
                        ret = 1;
                        break;

                    case SDLK_r: // A
                        joypad->action_direction[1] &= ~0x1;
                        ret = 1;
                        break;

                    case SDLK_v: // Start
                        joypad->action_direction[1] &= ~0x8;
                        ret = 1;
                        break;

                    case SDLK_b: // Select
                        joypad->action_direction[1] &= ~0x4;
                        ret = 1;
                        break;

                    default:
                        break;
                }
                break;
            
            case SDL_KEYUP:
                switch(event.key.keysym.sym){
                    case SDLK_w: // Up
                        joypad->action_direction[0] |= 0x4;
                        break;
                    
                    case SDLK_a: // Left
                        joypad->action_direction[0] |= 0x2;
                        break;

                    case SDLK_s: // Down
                        joypad->action_direction[0] |= 0x8;
                        break;

                    case SDLK_d: // Right
                        joypad->action_direction[0] |= 0x1;
                        break;

                    case SDLK_e: // B
                        joypad->action_direction[1] |= 0x2;
                        break;

                    case SDLK_r: // A
                        joypad->action_direction[1] |= 0x1;
                        break;

                    case SDLK_v: // Start
                        joypad->action_direction[1] |= 0x8;
                        break;

                    case SDLK_b: // Select
                        joypad->action_direction[1] |= 0x4;
                        break;

                    default:
                        break;
                    
                }
                break;

            default:
                break;
        }
    }

    return ret;
}