#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"

typedef enum INSTRUCTIONS
{
    ADD,ADDU,AND,NOR,OR,SLT,SLTU,SUB,SUBU,XOR, // rd, rs, rt type of instructions x < 10
    ADDI, ADDIU, ANDI, ORI, SLTI, SLTIU, XORI, // rt, rs, immediate type of instructions  9 < x < 17
    BEQ, BNE, //rs, rt, offset type of instructions 16 < x < 19
    BGEZ, BGEZAL, BGTZ, BLEZ, BLTZ, BLTZAL, //rs, offset type of instructions 18 < x < 25
    DIV, DIVU, MULT, MULTU, //rs, rt type of instruction 24 < x < 29
    LB, LBU, LH, LHU, LW, LWL, LWR, SB, SH, SW, SWL, SWR, //rt, offset(base) type of instruction 28 < x < 41
    JALR, JAL, JUMP, JR, // 40 < x < 45
    LUI,
    MFHI, MFLO, MTHI, MTLO,
    MTC0, MFC0, SLL,
    //NOT IMPLEMENTED YET
    SLLV, SRA, SRAV, SRL, SRLV, INST_SYSCALL


} INSTRUCTIONS;

extern const char* instruction_names[];
extern const char* cpu_registers[32];

typedef struct ps1_cpu ps1_cpu;
typedef struct log
{
    ps1_cpu* cpu;
} log;

void LOG(INSTRUCTIONS instr, ps1_cpu* cpu);

#endif