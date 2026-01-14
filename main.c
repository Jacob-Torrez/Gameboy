#include "cpu.h"

int main(){
    CPU* cpu_ptr = (CPU*)malloc(sizeof(CPU));
    cpu_ptr->mmu = (MMU*)malloc(sizeof(MMU));

    read_ROM(cpu_ptr->mmu, "Tetris.gb");

    return 0;
}