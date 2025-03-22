#include "bus.h"
#include "cpu.h"
#include "log.h"

#define RS (cpu->opcode >> 21) & 0x1F
#define RT (cpu->opcode >> 16) & 0x1F
#define RD (cpu->opcode >> 11) & 0x1F
#define BASE (cpu->opcode >> 21) & 0x1F
#define IMM16BITS (cpu->opcode & 0xFFFF)
#define OFFSET16BITS (cpu->opcode & 0xFFFF)
#define MASK26BITS (cpu->opcode & 0x3FFFFFF)
//Useful for the Address error exception to check if it is trying to read from outside KUSEG in user mode
#define valid_address(address) (address >= 0x00000000 && address <= 0x7FFFFFFF && (cpu->cop0[COP0_SR] & 2))

ps1_cpu* ps1_cpu_create()
{
    return (ps1_cpu*)malloc(sizeof(ps1_cpu));
}

void ps1_cpu_init(ps1_cpu* cpu)
{
    memset(cpu, 0, sizeof(ps1_cpu));
    cpu->pc = 0xbfc00000;
    cpu->log = fopen("log.txt", "w");
    log_add_fp(cpu->log, 0);
    log_set_quiet(true);
}

void ps1_cpu_destroy(ps1_cpu* cpu)
{
    free (cpu);
}

void ps1_connect_bus(ps1_bus* bus, ps1_cpu* cpu)
{
    cpu->bus = bus;
}

void cpu_tick(ps1_cpu* cpu)
{
    uint32_t pc = cpu->pc & 0x1FFFFFFF;  // Mask the PC to 0x1FFFFFFF to match the condition
    if ((pc == 0xA0 && cpu->r[9] == 0x3C) || (pc == 0xB0 && cpu->r[9] == 0x3D)) {
        char ch = (char)(cpu->r[4] & 0xFF);
        printf("%c", ch);
    }

    //TODO: Improve this to work properly and handle delay slot
    if(cpu->pc == 0x80030000)
        printf("executing shell code");

    if(cpu->branch)
    {
        cpu->opcode = ps1_bus_read_word(cpu->bus, cpu->pc);
        cpu_execute_instr(cpu);
        cpu->branch = false;   
        cpu->pc = cpu->branch_address;
    }
    else
    {
        cpu->opcode = ps1_bus_read_word(cpu->bus, cpu->pc);
        if(cpu->update_delay_load >= 0)
        {
            if(cpu->update_delay_load == 0)
                cpu->r[cpu->delay_load_register] = cpu->delay_load_value;
            cpu->update_delay_load--;
        }
        cpu_execute_instr(cpu);
        cpu->pc += 4;
    }
}

void cpu_execute_instr(ps1_cpu* cpu)
{
    switch((cpu->opcode & 0xFC000000) >> 26)
    {
        case (0b000000):
        {
            switch(cpu->opcode & 0x3F)
            {
                case 0b100000: cpu_execute_add(cpu); break;
                case 0b100001: cpu_execute_addu(cpu); break;
                case 0b100100: cpu_execute_and(cpu); break;
                case 0b001101: cpu_execute_break(cpu); break;
                case 0b011010: cpu_execute_div(cpu); break;
                case 0b011011: cpu_execute_divu(cpu); break;
                case 0b001001: cpu_execute_jalr(cpu); break;
                case 0b001000: cpu_execute_jr(cpu); break;
                case 0b010000: cpu_execute_mfhi(cpu); break;
                case 0b010010: cpu_execute_mflo(cpu); break;
                case 0b010001: cpu_execute_mthi(cpu); break;
                case 0b010011: cpu_execute_mtlo(cpu); break;
                case 0b011000: cpu_execute_mult(cpu); break;
                case 0b011001: cpu_execute_multu(cpu); break;
                case 0b100111: cpu_execute_nor(cpu); break;
                case 0b100101: cpu_execute_or(cpu); break;
                case 0b000000: cpu_execute_sll(cpu); break;
                case 0b000100: cpu_execute_sllv(cpu); break;
                case 0b101010: cpu_execute_slt(cpu); break;
                case 0b101011: cpu_execute_sltu(cpu); break;
                case 0b000011: cpu_execute_sra(cpu); break;
                case 0b000111: cpu_execute_srav(cpu); break;
                case 0b000010: cpu_execute_srl(cpu); break;
                case 0b000110: cpu_execute_srlv(cpu); break;
                case 0b100010: cpu_execute_sub(cpu); break;
                case 0b100011: cpu_execute_subu(cpu); break;
                case 0b001100: cpu_execute_syscall(cpu); break;
                case 0b100110: cpu_execute_xor(cpu); break;
            }
            break;
        }

        case (0b001000): cpu_execute_addi(cpu); break;
        case (0b001001): cpu_execute_addiu(cpu); break;
        case (0b001100): cpu_execute_andi(cpu); break;
        case (0b000100): cpu_execute_beq(cpu); break;
        case (0b000111): cpu_execute_bgtz(cpu); break;
        case (0b000110): cpu_execute_blez(cpu); break;
        case (0b000101): cpu_execute_bne(cpu); break;
        case (0b000010): cpu_execute_jump(cpu); break;
        case (0b000011): cpu_execute_jal(cpu); break;
        case (0b100000): cpu_execute_lb(cpu); break;
        case (0b100100): cpu_execute_lbu(cpu); break;
        case (0b100001): cpu_execute_lh(cpu); break;
        case (0b100101): cpu_execute_lhu(cpu); break;
        case (0b001111): cpu_execute_lui(cpu); break;
        case (0b100011): cpu_execute_lw(cpu); break;
        case (0b100010): cpu_execute_lwl(cpu); break;
        case (0b100110): cpu_execute_lwr(cpu); break;
        case (0b001101): cpu_execute_ori(cpu); break;
        case (0b101000): cpu_execute_sb(cpu); break;
        case (0b101001): cpu_execute_sh(cpu); break;
        case (0b001010): cpu_execute_slti(cpu); break;
        case (0b001011): cpu_execute_sltiu(cpu); break;
        case (0b101011): cpu_execute_sw(cpu); break;
        case (0b101010): cpu_execute_swl(cpu); break;
        case (0b101110): cpu_execute_swr(cpu); break;
        case (0b001110): cpu_execute_xori(cpu); break;

       case (0b000001):
       {
            switch((cpu->opcode & 0x001F0000) >> 16)
            {
                case (0b00001): cpu_execute_bgez(cpu);
                case (0b10001): cpu_execute_bgezal(cpu);
                case (0b00000): cpu_execute_bltz(cpu);
                case (0b10000): cpu_execute_bltzal(cpu);
            }
            break;
       }

       //COP0 instruction
       case (0b010000):
       {
            switch((cpu->opcode >> 21) & 0x1F)
            {
                case (0b00000): cpu_execute_mfc0(cpu);
                case (0b00100): cpu_execute_mtc0(cpu);
            }
       }
    }

    //TODO: Add later COP 2,3 instructions which are required for GTE and MDEC i believe
}

void cpu_handle_exception(ps1_cpu* cpu, EXCEPTION exception)
{
    //printf("Exceptions not handled yet! \n");
    //printf("Exception triggered: %i\n", exception);
    //TODO: add exception handling
}

void cpu_execute_add(ps1_cpu* cpu)
{
    int32_t value1 = (int32_t)cpu->r[RS];
    int32_t value2 = (int32_t)cpu->r[RT];
    int32_t result = value1 + value2;
    bool overflow = ((value1 ^ result) & (value2 ^ result)) >> 31;
    if(overflow)
        cpu_handle_exception(cpu, OVERFLOW);
    
    else
        cpu->r[RD] = (uint32_t)result;

    //log_trace("%08x: ADD", cpu->pc);
}

void cpu_execute_addi(ps1_cpu* cpu)
{
    int32_t imm = (int16_t)IMM16BITS;
    int32_t rs = (int32_t)cpu->r[RS];
    int32_t result = rs + imm;
    bool overflow = ((result ^ rs) & (result ^ imm)) >> 31;
    if(overflow)
        cpu_handle_exception(cpu, OVERFLOW);
    
    else
        cpu->r[RT] = (uint32_t)result;

    //log_trace("%08x: ADDI", cpu->pc);
}

void cpu_execute_addiu(ps1_cpu* cpu)
{
    int32_t imm = (int16_t)IMM16BITS;
    int32_t rs = (int32_t)cpu->r[RS];
    int32_t result = rs + imm;

    cpu->r[RT] = (uint32_t)result;
    //log_trace("%08x: ADDIU", cpu->pc);
}

void cpu_execute_addu(ps1_cpu* cpu)
{
    uint32_t result = cpu->r[RS] + cpu->r[RT];
    cpu->r[RD] = result;
    //log_trace("%08x: ADDU", cpu->pc);
}

void cpu_execute_and(ps1_cpu* cpu)
{
    uint32_t result = cpu->r[RS] & cpu->r[RT];
    cpu->r[RD] = result;
    //log_trace("%08x: AND", cpu->pc);
}

void cpu_execute_andi(ps1_cpu* cpu)
{
    uint32_t result = cpu->r[RS] & (uint32_t)IMM16BITS;
    cpu->r[RT] = result;
    //log_trace("%08x: ANDI", cpu->pc);
}

void cpu_execute_beq(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(cpu->r[RS] == cpu->r[RT])
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    //log_trace("%08x: BEQ", cpu->pc);
}

void cpu_execute_break(ps1_cpu* cpu)
{
    cpu_handle_exception(cpu, BREAK);
}

void cpu_execute_div(ps1_cpu* cpu)
{
    //TODO: check if division by zero has to be handled in any way
    cpu->lo = (int32_t)cpu->r[RS] / (int32_t)cpu->r[RT];
    cpu->hi = (int32_t)cpu->r[RS] % (int32_t)cpu->r[RT];
    //log_trace("%08x: DIV", cpu->pc);
}

void cpu_execute_divu(ps1_cpu* cpu)
{
    //TODO: check if division by zero has to be handled in any way
    cpu->lo = cpu->r[RS] / cpu->r[RT];
    cpu->hi = cpu->r[RS] % cpu->r[RT];
    //log_trace("%08x: DIVU", cpu->pc);
}

void cpu_execute_jalr(ps1_cpu* cpu)
{
    uint32_t target_address = cpu->r[RS];
    if((target_address & 0x3) != 0 || !valid_address(target_address))
    {
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    }
    cpu->branch_address = target_address;
    cpu->branch = true;
    cpu->r[RD] = cpu->pc + 8;
    //log_trace("%08x: JALR", cpu->pc);
}

void cpu_execute_jr(ps1_cpu* cpu)
{
    uint32_t target_address = cpu->r[RS];
    if((target_address & 0x3) != 0)
    {
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    }
    cpu->branch_address = target_address;
    cpu->branch = true;
    //log_trace("%08x: JR", cpu->pc);
}

void cpu_execute_mfhi(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->hi;
    //log_trace("%08x: MFHI", cpu->pc);
}

void cpu_execute_mflo(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->lo;
    //log_trace("%08x: MFLO", cpu->pc);
}

void cpu_execute_mthi(ps1_cpu* cpu)
{
    cpu->hi = cpu->r[RS];
    //log_trace("%08x: MTHI", cpu->pc);
}

void cpu_execute_mtlo(ps1_cpu* cpu)
{
    cpu->lo = cpu->r[RS];
    //log_trace("%08x: MTLO", cpu->pc);
}

void cpu_execute_mult(ps1_cpu* cpu)
{
    int64_t result = (int64_t)(int32_t)cpu->r[RS] * (int64_t)(int32_t)cpu->r[RT];
    cpu->lo = result & 0xFFFFFFFF;
    cpu->hi = (result >> 32) & 0xFFFFFFFF;
    //log_trace("%08x: MULT", cpu->pc);
}

void cpu_execute_multu(ps1_cpu* cpu)
{
    uint64_t result = cpu->r[RS] * cpu->r[RT];
    cpu->lo = result & 0xFFFFFFFF;
    cpu->hi = (result >> 32) & 0xFFFFFFFF;
    //log_trace("%08x: MULTU", cpu->pc);
}

void cpu_execute_nor(ps1_cpu* cpu)
{
    cpu->r[RD] = ~(cpu->r[RS] | cpu->r[RT]);
    //log_trace("%08x: NOR", cpu->pc);
}

void cpu_execute_or(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] | cpu->r[RT];
    //log_trace("%08x: OR", cpu->pc);
}

void cpu_execute_sll(ps1_cpu* cpu)
{                              //Extracting sa bits from opcode
    cpu->r[RD] = cpu->r[RT] << ((cpu->opcode >> 6) & 0x1F);
    //log_trace("%08x: SLL", cpu->pc);
}

void cpu_execute_sllv(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RT] << (cpu->r[RS] & 0x1F);
    //log_trace("%08x: SLLV", cpu->pc);
}

void cpu_execute_slt(ps1_cpu* cpu)
{
    cpu->r[RD] = ((int32_t)cpu->r[RS] < (int32_t)cpu->r[RT]) ? 1 : 0; 
    //log_trace("%08x: SLT", cpu->pc);
}

void cpu_execute_sltu(ps1_cpu* cpu)
{
    cpu->r[RD] = (cpu->r[RS] < cpu->r[RT]) ? 1 : 0;  
    //log_trace("%08x: SLTU", cpu->pc);
}

void cpu_execute_sra(ps1_cpu* cpu)
{
    cpu->r[RD] = (int32_t)(cpu->r[RT] >> ((cpu->opcode >> 6) & 0x1F));
    //log_trace("%08x: SRA", cpu->pc);
}

void cpu_execute_srav(ps1_cpu* cpu)
{
    cpu->r[RD] = (int32_t)(cpu->r[RT] >> (cpu->r[RS] & 0x1F));
    //log_trace("%08x: SRAV", cpu->pc);
}

void cpu_execute_srl(ps1_cpu* cpu)
{
    cpu->r[RD] = (cpu->r[RT] >> ((cpu->opcode >> 6) & 0x1F));  
    //log_trace("%08x: SRL", cpu->pc);
}

void cpu_execute_srlv(ps1_cpu* cpu)
{
    cpu->r[RD] = (cpu->r[RT] >> (cpu->r[RS] & 0x1F));
    //log_trace("%08x: SRLV", cpu->pc);
}

void cpu_execute_sub(ps1_cpu* cpu)
{
    int32_t value1 = (int32_t)cpu->r[RS];
    int32_t value2 = (int32_t)cpu->r[RT];
    int32_t result = value1 - value2;
    bool overflow = ((value1 ^ result) & (value2 ^ result)) >> 31;
    if(overflow)
        cpu_handle_exception(cpu, OVERFLOW);
    
    else
        cpu->r[RD] = (uint32_t)result;
    //log_trace("%08x: SUB", cpu->pc);
}

void cpu_execute_subu(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] - cpu->r[RT];
    //log_trace("%08x: SUBU", cpu->pc);
}

void cpu_execute_syscall(ps1_cpu* cpu)
{
    cpu_handle_exception(cpu, SYSCALL);
    //log_trace("%08x: SYSCALL", cpu->pc);
}

void cpu_execute_xor(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] ^ cpu->r[RT];
    //log_trace("%08x: XOR", cpu->pc);
}

void cpu_execute_bgtz(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(!(cpu->r[RS] & 0x80000000) && (cpu->r[RS] != 0))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    //log_trace("%08x: BGTZ", cpu->pc);
}

void cpu_execute_blez(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if((cpu->r[RS] & 0x80000000) || (cpu->r[RS] == 0))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    //log_trace("%08x: BLEZ", cpu->pc);
}

void cpu_execute_bne(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(cpu->r[RS] != cpu->r[RT])
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    ////log_trace("(PC: %08x)BNE %i, %i, %i branch address: %08x, rs = %08x, rt = %08x, offset = %08x", cpu->pc, RS, RT, OFFSET16BITS, cpu->branch_address, cpu->r[RS], cpu->r[RT], OFFSET16BITS);
    //log_trace("%08x: BNE", cpu->pc);
}

void cpu_execute_jump(ps1_cpu* cpu)
{
    cpu->branch_address = (MASK26BITS << 2) | ((cpu->pc + 4) & 0xF0000000);
    cpu->branch = true;
    //log_trace("%08x: JUMP", cpu->pc);
}

void cpu_execute_jal(ps1_cpu* cpu)
{
    cpu->branch_address = (MASK26BITS << 2) | ((cpu->pc + 4) & 0xF0000000);
    cpu->branch = true;
    cpu->r[31] = cpu->pc + 8;
    //log_trace("%08x: JAL", cpu->pc);
}

void cpu_execute_lb(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = (int32_t)(int8_t)ps1_bus_read_byte(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = 1;
    //log_trace("%08x: LB", cpu->pc);
}   

void cpu_execute_lbu(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = ps1_bus_read_byte(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = 1;
    //log_trace("%08x: LBU", cpu->pc);
}

void cpu_execute_lh(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!(virtual_address & 0x1) && !valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = (int32_t)(int16_t)ps1_bus_read_halfword(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = 1;
    //log_trace("%08x: LH", cpu->pc);
}

void cpu_execute_lhu(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!(virtual_address & 0x1) && !valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = ps1_bus_read_halfword(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = 1;
    //log_trace("%08x: LHU", cpu->pc);
}

void cpu_execute_lui(ps1_cpu* cpu)
{
    uint32_t result = IMM16BITS << 16;
    cpu->r[RT] = result;
    //log_trace("%08x: LUI", cpu->pc);
}

void cpu_execute_lw(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!(virtual_address & 0x3) && !valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = ps1_bus_read_word(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = 1;
    //log_trace("%08x: LW", cpu->pc);
}

void cpu_execute_lwl(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    uint32_t word = ps1_bus_read_word(cpu->bus, virtual_address & 0xFFFFFFFC);  //Get the full word to then extract the necessary bytes
    uint8_t shift = ((virtual_address & 0x3) << 3);
    uint32_t mask = 0x00FFFFFF >> shift;

    cpu->delay_load_value = (word << (24 - shift)) | (cpu->r[RT] & mask);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = 1;
    //log_trace("%08x: LWL", cpu->pc);
}

void cpu_execute_lwr(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    uint32_t word = ps1_bus_read_word(cpu->bus, virtual_address & 0xFFFFFFFC);  //Get the full word to then extract the necessary bytes
    uint8_t shift = ((virtual_address & 0x3) << 3);
    uint32_t mask = 0xFFFFFF00 >> (24 - shift);

    cpu->delay_load_value = (word >> shift) | (cpu->r[RT] & mask);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = 1;
    //log_trace("%08x: LWR", cpu->pc);
}

void cpu_execute_ori(ps1_cpu* cpu)
{
    cpu->r[RT] = ((uint32_t)IMM16BITS | cpu->r[RS]);
    //log_trace("%08x: ORI", cpu->pc);
}

void cpu_execute_sb(ps1_cpu* cpu)
{
    uint32_t virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    ps1_bus_store_byte(cpu->bus, virtual_address, (cpu->r[RT] & 0xFF));
    //log_trace("%08x: SB", cpu->pc);
}

void cpu_execute_sh(ps1_cpu* cpu)
{
    uint32_t virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    if(virtual_address & 0x1)
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    ps1_bus_store_halfword(cpu->bus, virtual_address, (cpu->r[RT] & 0xFFFF));
    //log_trace("%08x: SH", cpu->pc);
}

void cpu_execute_slti(ps1_cpu* cpu)
{
    cpu->r[RT] = ((int32_t)cpu->r[RS] < ((int32_t)(int16_t)IMM16BITS)) ? 1 : 0;
    //log_trace("%08x: SLTI", cpu->pc);
}


void cpu_execute_sltiu(ps1_cpu* cpu)
{
    cpu->r[RT] = (cpu->r[RS] < ((uint32_t)(int32_t)(int16_t)IMM16BITS)) ? 1 : 0;
    //log_trace("%08x: SLTIU", cpu->pc);
}

void cpu_execute_sw(ps1_cpu* cpu)
{
    uint32_t virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    if((virtual_address & 0x00) != 0)
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    ps1_bus_store_word(cpu->bus, virtual_address, cpu->r[RT]);
    //log_trace("%08x: SW", cpu->pc);
}

void cpu_execute_swl(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    uint32_t word = ps1_bus_read_word(cpu->bus, virtual_address & 0xFFFFFFFC);
    
    uint8_t shift = ((virtual_address & 0x3) << 3);
    uint32_t mask = 0xFFFFFF00 << shift;
    uint32_t new_word = (word & mask) | (cpu->r[RT] >> (24 - shift));
    ps1_bus_store_word (cpu->bus, virtual_address & 0xFFFFFFFC, new_word);
    //log_trace("%08x: SWL", cpu->pc);
}

void cpu_execute_swr(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    uint32_t word = ps1_bus_read_word(cpu->bus, virtual_address & 0xFFFFFFFC);
    
    uint8_t shift = ((virtual_address & 0x3) << 3);
    uint32_t mask = 0x00FFFFFF >> (24 - shift);
    uint32_t new_word = (word & mask) | (cpu->r[RT] << shift);
    ps1_bus_store_word (cpu->bus, virtual_address & 0xFFFFFFFC, new_word);
    //log_trace("%08x: SWR", cpu->pc);
}

void cpu_execute_xori(ps1_cpu* cpu)
{
    cpu->r[RT] = cpu->r[RS] ^ IMM16BITS;
    //log_trace("%08x: XORI", cpu->pc);
}

void cpu_execute_bgez(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(!(cpu->r[RS] & 0x80000000))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    //log_trace("%08x: BGEZ", cpu->pc);
}

void cpu_execute_bgezal(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    cpu->r[31] = cpu->pc + 8; //Address of the instruction after the delay slot
    if(!(cpu->r[RS] & 0x80000000))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    //log_trace("%08x: BGEZAL", cpu->pc);
}

void cpu_execute_bltz(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(cpu->r[RS] & 0x80000000)
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    //log_trace("%08x: BLTZ", cpu->pc);
}

void cpu_execute_bltzal(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    cpu->r[31] = cpu->pc + 8; //Address of the instruction after the delay slot
    if(cpu->r[RS] & 0x80000000)
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    //log_trace("%08x: BLTZAL", cpu->pc);
}

void cpu_execute_mfc0(ps1_cpu* cpu)
{
    cpu->r[RT] = cpu->cop0[RD];
    //log_trace("%08x: MFC0", cpu->pc);
}

void cpu_execute_mtc0(ps1_cpu* cpu)
{
    cpu->cop0[RD] = cpu->r[RT];  
    //log_trace("%08x: MTC0", cpu->pc);
}