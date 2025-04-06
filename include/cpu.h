#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "disassembler.h"

#define MAX_SIZE_FIFO 2

typedef enum EXCEPTION
 {
    OVERFLOW = 0x0C,
    BREAK = 0x9,
    ADEL = 0x04,
    ADES = 0x5,
    SYSCALL = 0x8

 } EXCEPTION;

 typedef enum {
    COP0_BPC = 3,      // Breakpoint on execute (R/W)
    COP0_BDA = 5,      // Breakpoint on data access (R/W)
    COP0_JUMPDEST = 6, // Randomly memorized jump address (R)
    COP0_DCIC = 7,     // Breakpoint control (R/W)
    COP0_BADVADDR = 8, // Bad Virtual Address (R)
    COP0_BDAM = 9,     // Data Access breakpoint mask (R/W)
    COP0_BPCM = 11,    // Execute breakpoint mask (R/W)
    COP0_SR = 12,      // System status register (R/W)
    COP0_CAUSE = 13,   // Exception cause register (R)
    COP0_EPC = 14,     // Exception return address (R)
    COP0_PRID = 15     // Processor ID (R)
} cop0_reg_t;

extern const uint32_t cpu_cop0_writemask[];

typedef struct ps1_bus ps1_bus;

typedef struct delayed_register
{
    uint32_t delayed_value;
    uint8_t delayed_register;
    bool modified;
    uint32_t pc;
} delayed_register;

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

typedef struct ps1_cpu
{
    uint32_t r[32]; //32 general registers. more info above
    uint32_t hi; //Special register hi
    uint32_t lo; //Special register lo
    uint32_t opcode; //All instructions are 32 bits long
    uint32_t pc; //Special register pc
    uint32_t cop0[32];
    ps1_bus* bus;
    delayed_register fifo_delay_load[MAX_SIZE_FIFO]; //FIFO that handles delay when loading values into general registers
    uint32_t virtual_address;

    //Useful for branches
    bool branch;
    uint32_t branch_address;
    bool branch_delay;
    
    //Useful for debugging
    FILE* log;
    FILE* exe;
    uint32_t debug_rs_value;
    uint32_t debug_rt_value;
    bool load_exe;
} ps1_cpu;

void cpu_tick(ps1_cpu* cpu);
void cpu_execute_instr(ps1_cpu* cpu);

//Cpu instructions
void cpu_execute_add(ps1_cpu* cpu);
void cpu_execute_addu(ps1_cpu* cpu);
void cpu_execute_and(ps1_cpu* cpu);
void cpu_execute_break(ps1_cpu* cpu);
void cpu_execute_div(ps1_cpu* cpu);
void cpu_execute_divu(ps1_cpu* cpu);
void cpu_execute_jalr(ps1_cpu* cpu);
void cpu_execute_jr(ps1_cpu* cpu);
void cpu_execute_mfhi(ps1_cpu* cpu);
void cpu_execute_mflo(ps1_cpu* cpu);
void cpu_execute_mthi(ps1_cpu* cpu);
void cpu_execute_mtlo(ps1_cpu* cpu);
void cpu_execute_mult(ps1_cpu* cpu);
void cpu_execute_multu(ps1_cpu* cpu);
void cpu_execute_nor(ps1_cpu* cpu);
void cpu_execute_or(ps1_cpu* cpu);
void cpu_execute_sll(ps1_cpu* cpu);
void cpu_execute_sllv(ps1_cpu* cpu);
void cpu_execute_slt(ps1_cpu* cpu);
void cpu_execute_sltu(ps1_cpu* cpu);
void cpu_execute_sra(ps1_cpu* cpu);
void cpu_execute_srav(ps1_cpu* cpu);
void cpu_execute_srl(ps1_cpu* cpu);
void cpu_execute_srlv(ps1_cpu* cpu);
void cpu_execute_sub(ps1_cpu* cpu);
void cpu_execute_subu(ps1_cpu* cpu);
void cpu_execute_syscall(ps1_cpu* cpu);
void cpu_execute_xor(ps1_cpu* cpu);

void cpu_execute_addi(ps1_cpu* cpu);
void cpu_execute_addiu(ps1_cpu* cpu);
void cpu_execute_addu(ps1_cpu* cpu);
void cpu_execute_andi(ps1_cpu* cpu);
void cpu_execute_beq(ps1_cpu* cpu);
void cpu_execute_bgtz(ps1_cpu* cpu);
void cpu_execute_blez(ps1_cpu* cpu);
void cpu_execute_bne(ps1_cpu* cpu);
void cpu_execute_jump(ps1_cpu* cpu);
void cpu_execute_jal(ps1_cpu* cpu);
void cpu_execute_lb(ps1_cpu* cpu);
void cpu_execute_lbu(ps1_cpu* cpu);
void cpu_execute_lh(ps1_cpu* cpu);
void cpu_execute_lhu(ps1_cpu* cpu);
void cpu_execute_lui(ps1_cpu* cpu);
void cpu_execute_lw(ps1_cpu* cpu);
void cpu_execute_lwl(ps1_cpu* cpu);
void cpu_execute_lwr(ps1_cpu* cpu);
void cpu_execute_ori(ps1_cpu* cpu);
void cpu_execute_sb(ps1_cpu* cpu);
void cpu_execute_sh(ps1_cpu* cpu);
void cpu_execute_slti(ps1_cpu* cpu);
void cpu_execute_sltiu(ps1_cpu* cpu);
void cpu_execute_sw(ps1_cpu* cpu);
void cpu_execute_swl(ps1_cpu* cpu);
void cpu_execute_swr(ps1_cpu* cpu);
void cpu_execute_xori(ps1_cpu* cpu);

void cpu_execute_bgez(ps1_cpu* cpu);
void cpu_execute_bgezal(ps1_cpu* cpu);
void cpu_execute_bltz(ps1_cpu* cpu);
void cpu_execute_bltzal(ps1_cpu* cpu);

//COP0 instructions
void cpu_execute_mfc0(ps1_cpu* cpu);
void cpu_execute_mtc0(ps1_cpu* cpu);
void cpu_execute_rfe(ps1_cpu* cpu);

void cpu_handle_exception(ps1_cpu* cpu, EXCEPTION exception);

ps1_cpu* ps1_cpu_create();
void ps1_cpu_init(ps1_cpu* cpu);
void ps1_cpu_destroy(ps1_cpu* cpu);
void ps1_connect_bus_cpu(ps1_bus* bus, ps1_cpu* cpu);

void sideload_exe(ps1_cpu* cpu);

#endif