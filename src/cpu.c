#include "bus.h"
#include "cpu.h"

#define RS (cpu->opcode >> 21) & 0x1F
#define RT (cpu->opcode >> 16) & 0x1F
#define RD (cpu->opcode >> 11) & 0x1F
#define BASE (cpu->opcode >> 21) & 0x1F
#define IMM16BITS (cpu->opcode & 0xFFFF)
#define OFFSET16BITS (cpu->opcode & 0xFFFF)
#define calc_delay_slot (cpu->delay_slot = (cpu->pc + 4))
#define MASK26BITS (cpu->opcode & 0x3FFFFFF)
//Useful for the Address error exception to check if it is trying to read from outside KUSEG in user mode
#define valid_address(address) (address >= 0x00000000 && address <= 0x7FFFFFFF && (cpu->cop0[COP0_SR] & 2))

ps1_cpu* ps1_cpu_create()
{
    (ps1_cpu*)malloc(sizeof(ps1_cpu));
}

void ps1_cpu_init(ps1_cpu* cpu)
{
    memset(cpu, 0, sizeof(ps1_cpu));
}

void ps1_cpu_destroy(ps1_cpu* cpu)
{
    free (cpu);
}

void cpu_tick(ps1_cpu* cpu)
{
    cpu->opcode = ps1_bus_read_word(cpu, cpu->pc);
    cpu_execute_instr(cpu);
    cpu->pc += 4;
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
    //Do something in the future
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
}

void cpu_execute_addiu(ps1_cpu* cpu)
{
    int32_t imm = (int16_t)IMM16BITS;
    int32_t rs = (int32_t)cpu->r[RS];
    int32_t result = rs + imm;

    cpu->r[RT] = (uint32_t)result;   
}

void cpu_execute_addu(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] + cpu->r[RT];
}

void cpu_execute_and(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] & cpu->r[RT];   
}

void cpu_execute_andi(ps1_cpu* cpu)
{
    cpu->r[RT] = cpu->r[RS] & IMM16BITS;   
}

void cpu_execute_beq(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offet, shifted )left wo bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    if(cpu->r[RS] == cpu->r[RT])
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
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
}

void cpu_execute_divu(ps1_cpu* cpu)
{
    //TODO: check if division by zero has to be handled in any way
    cpu->lo = cpu->r[RS] / cpu->r[RT];
    cpu->hi = cpu->r[RS] % cpu->r[RT];   
}

void cpu_execute_jalr(ps1_cpu* cpu)
{
    calc_delay_slot;
    uint32_t target_address = cpu->r[RS];
    if((target_address & 0x3) != 0 || !valid_address(target_address))
    {
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    }
    cpu->pc = cpu->r[RS];
    cpu->exec_delay_slot = true;
    cpu->r[RD] = cpu->delay_slot + 4;
}

void cpu_execute_jr(ps1_cpu* cpu)
{
    calc_delay_slot;
    uint32_t target_address = cpu->r[RS];
    if((target_address & 0x3) != 0)
    {
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    }
    cpu->pc = cpu->r[RS];
    cpu->exec_delay_slot = true;
}

void cpu_execute_mfhi(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->hi;
}

void cpu_execute_mflo(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->lo;
}

void cpu_execute_mthi(ps1_cpu* cpu)
{
    cpu->hi = cpu->r[RS];
}

void cpu_execute_mtlo(ps1_cpu* cpu)
{
    cpu->lo = cpu->r[RS];
}

void cpu_execute_mult(ps1_cpu* cpu)
{
    uint32_t result = (int32_t)cpu->r[RS] * (int32_t)cpu->r[RT];
    cpu->lo = result & 0xFFFF;
    cpu->hi = (result >> 16) & 0xFFFF;
}

void cpu_execute_multu(ps1_cpu* cpu)
{
    uint32_t result = cpu->r[RS] * cpu->r[RT];
    cpu->lo = result & 0xFFFF;
    cpu->hi = (result >> 16) & 0xFFFF;
}

void cpu_execute_nor(ps1_cpu* cpu)
{
    cpu->r[RD] = ~(cpu->r[RS] | cpu->r[RT]);
}

void cpu_execute_or(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] | cpu->r[RT];
}

void cpu_execute_sll(ps1_cpu* cpu)
{                              //Extracting sa bits from opcode
    cpu->r[RD] = cpu->r[RT] << ((cpu->opcode >> 6) & 0x1F);
}

void cpu_execute_sllv(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RT] << (cpu->r[RS] & 0x1F);
}

void cpu_execute_slt(ps1_cpu* cpu)
{
    cpu->r[RD] = ((int32_t)cpu->r[RS] < (int32_t)cpu->r[RT]) ? 1 : 0;  
}

void cpu_execute_sltu(ps1_cpu* cpu)
{
    cpu->r[RD] = (cpu->r[RS] < cpu->r[RT]) ? 1 : 0;  
}

void cpu_execute_sra(ps1_cpu* cpu)
{
    cpu->r[RD] = (int32_t)(cpu->r[RT] >> ((cpu->opcode >> 6) & 0x1F));
}

void cpu_execute_srav(ps1_cpu* cpu)
{
    cpu->r[RD] = (int32_t)(cpu->r[RT] >> (cpu->r[RS] & 0x1F));
}

void cpu_execute_srl(ps1_cpu* cpu)
{
    cpu->r[RD] = (cpu->r[RT] >> ((cpu->opcode >> 6) & 0x1F));  
}

void cpu_execute_srlv(ps1_cpu* cpu)
{
    cpu->r[RD] = (cpu->r[RT] >> (cpu->r[RS] & 0x1F));
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
}

void cpu_execute_subu(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] - cpu->r[RT];
}

void cpu_execute_syscall(ps1_cpu* cpu)
{
    cpu_handle_exception(cpu, SYSCALL);
}

void cpu_execute_xor(ps1_cpu* cpu)
{
    cpu->r[RD] = cpu->r[RS] ^ cpu->r[RT];
}

void cpu_execute_bgtz(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    if(!(cpu->r[RS] & 0x80000000) && (cpu->r[RS] != 0))
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
}

void cpu_execute_blez(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    if((cpu->r[RS] & 0x80000000) || (cpu->r[RS] == 0))
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
}

void cpu_execute_bne(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    if(cpu->r[RS] != cpu->r[RT])
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
}

void cpu_execute_jump(ps1_cpu* cpu)
{
    calc_delay_slot;
    cpu->pc = (MASK26BITS << 2) | (cpu->delay_slot & 0xF0000000);
    cpu->exec_delay_slot = true;
}

void cpu_execute_jal(ps1_cpu* cpu)
{
    calc_delay_slot;
    cpu->pc = (MASK26BITS << 2) | (cpu->delay_slot & 0xF0000000);
    cpu->exec_delay_slot = true;
    cpu->r[31] = cpu->delay_slot + 4;
}

void cpu_execute_lb(ps1_cpu* cpu)
{
    int32_t offset = (uint16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = (int32_t)(int8_t)ps1_bus_read_byte(cpu->bus, virtual_address);

    cpu->delay_load_register = RT;
    cpu->update_delay_load = true;
}   

void cpu_execute_lbu(ps1_cpu* cpu)
{
    int32_t offset = (uint16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = ps1_bus_read_byte(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = true;
}

void cpu_execute_lh(ps1_cpu* cpu)
{
    int32_t offset = (uint16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!(virtual_address & 0x1) && !valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = (int32_t)(int16_t)ps1_bus_read_halfword(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = true;
}

void cpu_execute_lhu(ps1_cpu* cpu)
{
    int32_t offset = (uint16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!(virtual_address & 0x1) && !valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = ps1_bus_read_halfword(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = true;
}

void cpu_execute_lui(ps1_cpu* cpu)
{
    uint32_t result = IMM16BITS << 16;
    cpu->delay_load_value = result;
    cpu->delay_load_register = RT;
    cpu->update_delay_load = true;
}

void cpu_execute_lw(ps1_cpu* cpu)
{
    int32_t offset = (int16_t)OFFSET16BITS;
    int32_t virtual_address = offset + cpu->r[BASE];
    if(!(virtual_address & 0x3) && !valid_address(virtual_address))
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    cpu->delay_load_value = ps1_bus_read_word(cpu->bus, virtual_address);
    cpu->delay_load_register = RT;
    cpu->update_delay_load = true;
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
    cpu->update_delay_load = true;
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
    cpu->update_delay_load = true;
}

void cpu_execute_ori(ps1_cpu* cpu)
{
    cpu->r[RT] = ((uint32_t)IMM16BITS | cpu->r[RS]);
}

void cpu_execute_sb(ps1_cpu* cpu)
{
    uint32_t virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    ps1_bus_store_byte(cpu->bus, virtual_address, (cpu->r[RT] & 0xFF));
}

void cpu_execute_sh(ps1_cpu* cpu)
{
    uint32_t virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    if(virtual_address & 0x1)
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    ps1_bus_store_halfword(cpu->bus, virtual_address, (cpu->r[RT] & 0xFFFF));
}

void cpu_execute_slti(ps1_cpu* cpu)
{
    cpu->r[RT] = (cpu->r[RS] < ((int32_t)(int16_t)IMM16BITS)) ? 1 : 0;
}


void cpu_execute_sltiu(ps1_cpu* cpu)
{
    cpu->r[RT] = (cpu->r[RS] < ((uint32_t)(int32_t)(int16_t)IMM16BITS)) ? 1 : 0;
}

void cpu_execute_sw(ps1_cpu* cpu)
{
    uint32_t virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    if(virtual_address & 0x00 != 0)
        cpu_handle_exception(cpu, ADDRESS_ERROR);
    ps1_bus_store_word(cpu->bus, virtual_address, cpu->r[RT]);
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
}

void cpu_execute_xori(ps1_cpu* cpu)
{
    cpu->r[RT] = cpu->r[RS] ^ IMM16BITS;
}

void cpu_execute_bgez(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    if(cpu->r[RS] & 0x80000000)
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
}

void cpu_execute_bgezal(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    cpu->r[31] = cpu->delay_slot + 4; //Address of the instruction after the delay slot
    if(!(cpu->r[RS] & 0x80000000))
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
}

void cpu_execute_bltz(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    if(cpu->r[RS] & 0x80000000)
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
}

void cpu_execute_bltzal(ps1_cpu* cpu)
{
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    calc_delay_slot;
    uint32_t target_address = offset + cpu->delay_slot;
    cpu->r[31] = cpu->delay_slot + 4; //Address of the instruction after the delay slot
    if(cpu->r[RS] & 0x80000000)
    {
        cpu->pc = target_address;
        cpu->exec_delay_slot = true;
    }
}

void cpu_execute_mfc0(ps1_cpu* cpu)
{
    cpu->r[RT] = cpu->cop0[RD];
}

void cpu_execute_mtc0(ps1_cpu* cpu)
{
    cpu->cop0[RD] = cpu->r[RT];  
}