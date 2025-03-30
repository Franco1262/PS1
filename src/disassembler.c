#include "cpu.h"
#include "disassembler.h"

#define RS (cpu->opcode >> 21) & 0x1F
#define RT (cpu->opcode >> 16) & 0x1F
#define RD (cpu->opcode >> 11) & 0x1F
#define BASE (cpu->opcode >> 21) & 0x1F
#define IMM16BITS (cpu->opcode & 0xFFFF)
#define OFFSET16BITS (cpu->opcode & 0xFFFF)
#define MASK26BITS (cpu->opcode & 0x3FFFFFF)
#define IMM5BITS ((cpu->opcode >> 6) & 0x1F)
#define SA (cpu->opcode & 0x1F)

const char* instruction_names[] = {
    "ADD", "ADDU", "AND", "NOR", "OR", "SLT", "SLTU", "SUB", "SUBU", "XOR",
    "ADDI", "ADDIU", "ANDI", "ORI", "SLTI", "SLTIU", "XORI",
    "BEQ", "BNE",
    "BGEZ", "BGEZAL", "BGTZ", "BLEZ", "BLTZ", "BLTZAL",
    "DIV", "DIVU", "MULT", "MULTU",
    "LB", "LBU", "LH", "LHU", "LW", "LWL", "LWR", "SB", "SH", "SW", "SWL", "SWR",
    "JALR", "JAL", "J", "JR", "LUI", "MFHI", "MFLO", "MTHI", "MTLO", "MTC0", "MFC0",
    "SLL"
};

const char* cpu_registers[32] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

void LOG(INSTRUCTIONS instr, ps1_cpu* cpu)
{
    switch (instr) 
    {
        // Instructions 0 - 9
        case ADD: case ADDU: case AND: case NOR: case OR:
        case SLT: case SLTU: case SUB: case SUBU: case XOR:
            log_trace("%08x: %-5s, rd:%-4s rs:%-4s, rt:%-5s, result:%08x     ; %-4s: %08x %-4s: %08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RD], cpu_registers[RS], cpu_registers[RT], cpu->r[RD], cpu_registers[RS], 
                cpu->debug_rs_value, cpu_registers[RT], cpu->debug_rt_value);
            break;
        
        // Instructions 10 - 16
        case ADDI: case ADDIU: case ANDI: case ORI: case SLTI:
        case SLTIU: case XORI:
            log_trace("%08x: %-5s, rt:%-4s rs:%-4s, imm:%04x, result:%08x     ; %-4s: %08x imm: %04x", 
                cpu->pc, instruction_names[instr], cpu_registers[RT], cpu_registers[RS], IMM16BITS, cpu->r[RT],
                cpu_registers[RS], cpu->debug_rs_value, IMM16BITS);
            break;
        
        // Instructions 17 - 18
        case BEQ: case BNE:
            log_trace("%08x: %-5s, rs:%-4s rt:%-4s, off:%04x, branch address:%08x     ;%-5s:%08x %-5s:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RS], cpu_registers[RT], OFFSET16BITS, cpu->branch_address, cpu_registers[RS], cpu->debug_rs_value, cpu_registers[RT], cpu->debug_rt_value);           
            break;
        
        // Instructions 19 - 24
        case BGEZ: case BGEZAL: case BGTZ: case BLEZ: case BLTZ: case BLTZAL:
            log_trace("%08x: %-5s, rs:%-4s, off:%04x, branch address:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RS], OFFSET16BITS, cpu->branch_address); 
            break;
        
        // Instructions 25 - 28
        case DIV: case DIVU: case MULT: case MULTU:
            log_trace("%08x: %-5s, rs:%-4s, rt:%-5s, hi:%08x, lo:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RS], cpu_registers[RT], cpu->hi, cpu->lo);
            break;
        
        // Instructions 29 - 40
        case LB: case LBU: case LH: case LHU: case LW:
        case LWL: case LWR: case SB: case SH: case SW:
        case SWL: case SWR:
            log_trace("%08x: %-5s, rt:%-5s, %04x(%05s)     ; addr:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RT], OFFSET16BITS, cpu_registers[BASE], cpu->virtual_address);
            break;
        case JALR: case JAL: case JUMP: case JR:
            log_trace("%08x: %-5s, addr: %08x", 
                cpu->pc, instruction_names[instr], cpu->branch_address);
            break;
        case LUI:
            log_trace("%08x: %-5s, rt:%-4s, imm:%04x, result:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RT], IMM16BITS, cpu->r[RT]); 
            break;
        case MFHI:
            log_trace("%08x: %-5s, rd:%-4s, hi:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RD], cpu->hi); 
            break;
        case MFLO:
            log_trace("%08x: %-5s, rd:%-4s, lo:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RD], cpu->lo);
        case MTHI:
            log_trace("%08x: %-5s, rs:%-4s, hi:%08x     ; %-5s:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RS], cpu->hi, cpu_registers[RS], cpu->debug_rs_value); 
        break;
        case MTLO:
            log_trace("%08x: %-5s, rs:%-4s, lo:%08x     ; %-5s:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RS], cpu->lo, cpu_registers[RS], cpu->debug_rs_value);
            break;
        case MTC0:
            log_trace("%08x: %-5s, rt:%-4s, rd:cop%08x     ; %-5s:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RT], RD, cpu_registers[RT], cpu->debug_rt_value);           
            break;
        case SLL:
            log_trace("%08x: %-5s, rd:%-4s rt:%-4s, sa:%02x, result:%08x", 
                cpu->pc, instruction_names[instr], cpu_registers[RD], cpu_registers[RT], SA, cpu->r[RD]);           
            break;
                          
        default:

            break;
    }
}