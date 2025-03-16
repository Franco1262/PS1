#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


/*
 Name       Alias    Common Usage
  R0         zero     Constant (always 0)
  R1         at       Assembler temporary (destroyed by some assembler pseudoinstructions!)
  R2-R3      v0-v1    Subroutine return values, may be changed by subroutines
  R4-R7      a0-a3    Subroutine arguments, may be changed by subroutines
  R8-R15     t0-t7    Temporaries, may be changed by subroutines
  R16-R23    s0-s7    Static variables, must be saved by subs
  R24-R25    t8-t9    Temporaries, may be changed by subroutines
  R26-R27    k0-k1    Reserved for kernel (destroyed by some IRQ handlers!)
  R28        gp       Global pointer (rarely used)
  R29        sp       Stack pointer
  R30        fp(s8)   Frame Pointer, or 9th Static variable, must be saved
  R31        ra       Return address (used so by JAL,BLTZAL,BGEZAL opcodes)
  -          pc       Program counter
  -          hi,lo    Multiply/divide results, may be changed by subroutines 
 */

typedef struct cpu_ps1
{
    uint32_t r[32];
    uint32_t opcode; //All instructions are 32 bit long
    uint32_t pc;
    uint8_t cycles;
} cpu_ps1;

void cpu_execute_instr(cpu_ps1* cpu);

//Cpu instructions
void cpu_execute_add(cpu_ps1* cpu);
void cpu_execute_addu(cpu_ps1* cpu);
void cpu_execute_and(cpu_ps1* cpu);
void cpu_execute_break(cpu_ps1* cpu);
void cpu_execute_div(cpu_ps1* cpu);
void cpu_execute_divu(cpu_ps1* cpu);
void cpu_execute_jalr(cpu_ps1* cpu);
void cpu_execute_jr(cpu_ps1* cpu);
void cpu_execute_mfhi(cpu_ps1* cpu);
void cpu_execute_mflo(cpu_ps1* cpu);
void cpu_execute_mthi(cpu_ps1* cpu);
void cpu_execute_mtlo(cpu_ps1* cpu);
void cpu_execute_mult(cpu_ps1* cpu);
void cpu_execute_multu(cpu_ps1* cpu);
void cpu_execute_nor(cpu_ps1* cpu);
void cpu_execute_or(cpu_ps1* cpu);
void cpu_execute_sll(cpu_ps1* cpu);
void cpu_execute_sllv(cpu_ps1* cpu);
void cpu_execute_slt(cpu_ps1* cpu);
void cpu_execute_sltu(cpu_ps1* cpu);
void cpu_execute_sra(cpu_ps1* cpu);
void cpu_execute_srav(cpu_ps1* cpu);
void cpu_execute_srl(cpu_ps1* cpu);
void cpu_execute_srlv(cpu_ps1* cpu);
void cpu_execute_sub(cpu_ps1* cpu);
void cpu_execute_subu(cpu_ps1* cpu);
void cpu_execute_syscall(cpu_ps1* cpu);
void cpu_execute_xor(cpu_ps1* cpu);

void cpu_execute_addi(cpu_ps1* cpu);
void cpu_execute_andi(cpu_ps1* cpu);
void cpu_execute_beq(cpu_ps1* cpu);
void cpu_execute_bgtz(cpu_ps1* cpu);
void cpu_execute_blez(cpu_ps1* cpu);
void cpu_execute_bne(cpu_ps1* cpu);
void cpu_execute_jump(cpu_ps1* cpu);
void cpu_execute_jal(cpu_ps1* cpu);
void cpu_execute_lb(cpu_ps1* cpu);
void cpu_execute_lbu(cpu_ps1* cpu);
void cpu_execute_lh(cpu_ps1* cpu);
void cpu_execute_lhu(cpu_ps1* cpu);
void cpu_execute_lui(cpu_ps1* cpu);
void cpu_execute_lw(cpu_ps1* cpu);
void cpu_execute_lwl(cpu_ps1* cpu);
void cpu_execute_lwr(cpu_ps1* cpu);
void cpu_execute_ori(cpu_ps1* cpu);
void cpu_execute_sb(cpu_ps1* cpu);
void cpu_execute_sh(cpu_ps1* cpu);
void cpu_execute_slti(cpu_ps1* cpu);
void cpu_execute_sltiu(cpu_ps1* cpu);
void cpu_execute_sw(cpu_ps1* cpu);
void cpu_execute_swl(cpu_ps1* cpu);
void cpu_execute_swr(cpu_ps1* cpu);
void cpu_execute_xori(cpu_ps1* cpu);

void cpu_execute_bgez(cpu_ps1* cpu);
void cpu_execute_bgezal(cpu_ps1* cpu);
void cpu_execute_bltz(cpu_ps1* cpu);
void cpu_execute_bltzal(cpu_ps1* cpu);

//COP0 instructions
void cpu_execute_mfc0(cpu_ps1* cpu);
void cpu_execute_mtc0(cpu_ps1* cpu);

#endif