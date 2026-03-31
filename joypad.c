#include "joypad.h"

uint8_t process_events(JOYPAD* joypad){
    uint8_t ret = 0;

    SDL_Event event;

    while (SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym){
                    case SDLK_w: // Up
                        break;
                    
                    case SDLK_a: // Left
                        break;

                    case SDLK_s: // Down
                        break;

                    case SDLK_d: // Right
                        break;

                    case SDLK_e: // B
                        break;

                    case SDLK_r: // A
                        break;

                    case SDLK_v: // Start
                        break;

                    case SDLK_b: // Select
                        break;

                    default:
                        break;
                    
                }
                break;
            
            case SDL_KEYUP:
                switch(event.key.keysym.sym){
                    case SDLK_w: // Up
                        break;
                    
                    case SDLK_a: // Left
                        break;

                    case SDLK_s: // Down
                        break;

                    case SDLK_d: // Right
                        break;

                    case SDLK_e: // B
                        break;

                    case SDLK_r: // A
                        break;

                    case SDLK_v: // Start
                        break;

                    case SDLK_b: // Select
                        break;
                    
                }

                ret = 0;
                break;

            default:
                ret = 0;
                break;
        }
    }
}