#ifndef CPU_U
#define CPU_H

#include <stdint.h>

typedef struct {
    uint8_t A, B, C, D, E, H, L, F;
    uint16_t SP, PC;
} CPU;

typedef void (*OpHandler)(CPU* cpu);

void cpu_reset(CPU* cpu);
void cpu_step(CPU* cpu);
void cpu_execute(CPU* cpu);

void nop_0x00(CPU* cpu);
void ld_bc_d16_0x01(CPU* cpu);
void ld_bc_ptr_a_0x02(CPU* cpu);
void inc_bc_0x03(CPU* cpu);
void inc_b_0x04(CPU* cpu);
void dec_b_0x05(CPU* cpu);
void ld_b_d8_0x06(CPU* cpu);
void rlca_0x07(CPU* cpu);
void ld_a16_ptr_sp_0x08(CPU* cpu);
void add_hl_bc_0x09(CPU* cpu);
void ld_a_bc_ptr_0x0A(CPU* cpu);
void dec_bc_0x0B(CPU* cpu);
void inc_c_0x0C(CPU* cpu);
void dec_c_0x0D(CPU* cpu);
void ld_c_d8_0x0E(CPU* cpu);
void rrca0x0F(CPU* cpu);
void stop_0x10(CPU* cpu); // 2 bytes
void ld_de_d16_0x11(CPU* cpu);
void ld_de_ptr_a_0x12(CPU* cpu);
void inc_de_0x13(CPU* cpu);
void inc_d_0x14(CPU* cpu);
void dec_d_0x15(CPU* cpu);
void ld_d_d8_0x16(CPU* cpu);
void rla_0x17(CPU* cpu);
void jr_s8_0x18(CPU* cpu);
void add_hl_de_0x19(CPU* cpu);
void ld_a_de_ptr_0x1A(CPU* cpu);
void dec_de_0x1B(CPU* cpu);
void inc_e_0x1C(CPU* cpu);
void dec_e_0x1D(CPU* cpu);
void ld_e_d8_0x1E(CPU* cpu);
void rra_0x1F(CPU* cpu);
void jr_nz_s8_0x20(CPU* cpu);
void ld_hl_d16_0x21(CPU* cpu);
void ld_hli_ptr_a_0x22(CPU* cpu);
void inc_hl_0x23(CPU* cpu);
void inc_h_0x24(CPU* cpu);
void dec_h_0x25(CPU* cpu);
void ld_h_d8_0x26(CPU* cpu);
void daa_0x27(CPU* cpu);
void jr_z_s8_0x28(CPU* cpu);
void add_hl_hl_0x29(CPU* cpu);
void ld_a_hli_ptr_0x2A(CPU* cpu);
void dec_hl_0x2B(CPU* cpu);
void inc_l_0x2C(CPU* cpu);
void dec_l_0x2D(CPU* cpu);
void ld_l_d8_0x2E(CPU* cpu);
void cpl_0x2F(CPU* cpu);


#endif