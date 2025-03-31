#include "ram.h"
#include "bus.h"
#include "cpu.h"

#define RS (cpu->opcode >> 21) & 0x1F
#define RT (cpu->opcode >> 16) & 0x1F
#define RD (cpu->opcode >> 11) & 0x1F
#define BASE (cpu->opcode >> 21) & 0x1F
#define IMM16BITS (cpu->opcode & 0xFFFF)
#define OFFSET16BITS (cpu->opcode & 0xFFFF)
#define MASK26BITS (cpu->opcode & 0x3FFFFFF)
#define IMM5BITS ((cpu->opcode >> 6) & 0x1F)
//Useful for the Address error exception to check if it is trying to read from outside KUSEG in user mode
#define valid_address(address) (address >= 0x00000000 && address <= 0x7FFFFFFF && (cpu->cop0[COP0_SR] & 2))
#define HANDLE_BRANCH \
            if(cpu->branch) \
            { \
                if(cpu->branch_delay == true)\
                    cpu->branch_delay = false;\
                else\
                {\
                    cpu->branch = false;\
                    cpu->pc = cpu->branch_address;\
                    cpu->branch_delay = true;\
                }\
            }
//Handles delay loads by using a fifo that waits for the next instruction to be executed to update the value
#define HANDLE_LOAD\
            if(cpu->fifo_delay_load[1].modified)\
            {\
                if(cpu->fifo_delay_load[0].delayed_register != cpu->fifo_delay_load[1].delayed_register)\
                {\
                    cpu->r[cpu->fifo_delay_load[1].delayed_register] = cpu->fifo_delay_load[1].delayed_value;\
                }\
                cpu->fifo_delay_load[1].modified = false; /* Reset old entry */ \
            }\
            if(cpu->fifo_delay_load[0].modified)\
            {\
                cpu->fifo_delay_load[1] = cpu->fifo_delay_load[0];\
                cpu->fifo_delay_load[0] = (delayed_register){0};\
            }\
            cpu->r[0] = 0;

//useful for getting output from the bios and .exes
#define tty_output(pc)\
            if ((pc == 0xA0 && cpu->r[9] == 0x3C) || (pc == 0xB0 && cpu->r[9] == 0x3D))\
            {\
                char ch = (char)(cpu->r[4] & 0xFF);\
                printf("%c", ch);\
            }



const uint32_t cpu_cop0_writemask[] = {
    0x00000000, // cop0r0   - N/A
    0x00000000, // cop0r1   - N/A
    0x00000000, // cop0r2   - N/A
    0xffffffff, // BPC      - Breakpoint on execute (R/W)
    0x00000000, // cop0r4   - N/A
    0xffffffff, // BDA      - Breakpoint on data access (R/W)
    0x00000000, // JUMPDEST - Randomly memorized jump address (R)
    0xffc0f03f, // DCIC     - Breakpoint control (R/W)
    0x00000000, // BadVaddr - Bad Virtual Address (R)
    0xffffffff, // BDAM     - Data Access breakpoint mask (R/W)
    0x00000000, // cop0r10  - N/A
    0xffffffff, // BPCM     - Execute breakpoint mask (R/W)
    0xffffffff, // SR       - System status register (R/W)
    0x00000300, // CAUSE    - Describes the most recently recognised exception (R)
    0x00000000, // EPC      - Return Address from Trap (R)
    0x00000000  // PRID     - Processor ID (R)
};

ps1_cpu* ps1_cpu_create()
{
    return (ps1_cpu*)malloc(sizeof(ps1_cpu));
}

void ps1_cpu_init(ps1_cpu* cpu)
{
    memset(cpu, 0, sizeof(ps1_cpu));
    cpu->pc = 0xbfc00000;
    cpu->branch_delay = true;
    cpu->load_exe = true;

    //Init logging
    cpu->log = fopen("log.txt", "w");
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

void sideload_exe(ps1_cpu* cpu) 
{
    FILE *file = fopen("psx_tests/psxtest_cpu.exe", "rb");
    if (!file) 
    {
        perror("Failed to open EXE");
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate memory for EXE file
    uint8_t *exe = malloc(file_size);
    if (!exe) 
    {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    // Read EXE file into memory
    if (fread(exe, 1, file_size, file) != file_size) 
    {
        perror("Failed to read EXE file");
        free(exe);
        fclose(file);
        return;
    }
    fclose(file);

    // Parse EXE header
    cpu->pc = *(uint32_t*)(exe + 0x10);
    cpu->r[28] = *(uint32_t*)(exe + 0x14);
    uint32_t exe_ram_address = (*(uint32_t *)(exe + 0x18)) & 0x1FFFFF;
    uint32_t exe_size_2kb = *(uint32_t *)(exe + 0x1C);
    uint32_t initial_sp = *(uint32_t *)(exe + 0x30);

    // Debugging output
/*     printf("PC: %08X\n", cpu->pc);
    printf("R28: %08X\n", cpu->r[28]);
    printf("Raw EXE RAM Address: %08X\n", *(uint32_t *)(exe + 0x18));
    printf("Final RAM Address (Masked): %08X\n", exe_ram_address);
    printf("EXE Size (2KB units): %i\n", exe_size_2kb);
    printf("Stack Pointer (R29, R30): %08X\n", initial_sp); */

    // Copy EXE data into RAM
    memcpy(ps1_bus_get_ram(cpu->bus)->ram_buff + exe_ram_address, exe + 2048, exe_size_2kb);

    // Set stack pointer only if it's non-zero
    if (initial_sp) 
    {
        cpu->r[29] = initial_sp;
        cpu->r[30] = initial_sp;
    }

    // Cleanup
    free(exe);
    //log_add_fp(cpu->log, 0);
}



void cpu_tick(ps1_cpu* cpu)
{
    tty_output((cpu->pc & 0x1FFFFFFF));

    if(cpu->pc == 0x80030000 && cpu->load_exe)
    {
        sideload_exe(cpu);
        cpu->load_exe = false;
    }
    if(cpu->pc & 0x3)
        cpu_handle_exception(cpu, ADEL);
    else
    {
        HANDLE_LOAD;
        cpu->opcode = ps1_bus_read_word(cpu->bus, cpu->pc);
        cpu_execute_instr(cpu);
        cpu->pc += 4;
        HANDLE_BRANCH;
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
                case (0b00001): cpu_execute_bgez(cpu); break;
                case (0b10001): cpu_execute_bgezal(cpu); break;
                case (0b00000): cpu_execute_bltz(cpu); break;
                case (0b10000): cpu_execute_bltzal(cpu); break;
            }
            break;
       }

       //COP0 instruction
       case (0b010000):
       {
            switch((cpu->opcode >> 21) & 0x1F)
            {
                case (0b00000): cpu_execute_mfc0(cpu); break;
                case (0b00100): cpu_execute_mtc0(cpu); break;
            }
            break;
       }

       default:
            break;
    }

    //TODO: Add later COP 2,3 instructions which are required for GTE and MDEC i believe
}

void cpu_handle_exception(ps1_cpu* cpu, EXCEPTION exception)
{
    cpu->cop0[COP0_CAUSE] = (cpu->cop0[COP0_CAUSE] & 0xFFFFFF00) | (exception << 2);
    cpu->cop0[COP0_EPC] = cpu->pc;

    
    if(cpu->branch && !cpu->branch_delay) // if executing the delay slot
    {
        cpu->cop0[COP0_EPC] = cpu->pc - 4;
        cpu->cop0[COP0_CAUSE] |= 0x80000000;
    }

    cpu->pc = ((cpu->cop0[COP0_SR] & 0x00400000) ? 0xBFC00180 : 0x80000080) - 4;
    log_trace("EXCEPTION");
}

void cpu_execute_add(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];

    int32_t value1 = cpu->r[RS];
    int32_t value2 = cpu->r[RT];
    int32_t result = value1 + value2;
    bool overflow = (((value1 ^ result) & (value2 ^ result)) >> 31) & 0x1;

    if(overflow)
    {
        cpu_handle_exception(cpu, OVERFLOW);
    }

     
    else
        cpu->r[RD] = result;

    LOG(ADD, cpu);
}

void cpu_execute_addi(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t imm = (int32_t)(int16_t)IMM16BITS;
    int32_t rs = cpu->r[RS];
    int32_t result = rs + imm;
    bool overflow = (((result ^ rs) & (result ^ imm)) >> 31) & 0x1;

    if(overflow)
        cpu_handle_exception(cpu, OVERFLOW);
    
    else
        cpu->r[RT] = result;
    LOG(ADDI, cpu);
}

void cpu_execute_addiu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];

    uint32_t imm = (int32_t)(int16_t)IMM16BITS;
    cpu->r[RT] = cpu->r[RS] + imm;
    LOG(ADDIU, cpu);
}

void cpu_execute_addu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];

    cpu->r[RD] = cpu->r[RS] + cpu->r[RT];
    LOG(ADDU, cpu);
}

void cpu_execute_and(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = cpu->r[RS] & cpu->r[RT];
    LOG(AND, cpu);
}

void cpu_execute_andi(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RT] = cpu->r[RS] & (uint32_t)IMM16BITS;
    LOG(ANDI, cpu);
}

void cpu_execute_beq(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];

    uint32_t offset = ((int32_t)(int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(cpu->r[RS] == cpu->r[RT])
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    LOG(BEQ, cpu);
}

void cpu_execute_break(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu_handle_exception(cpu, BREAK);
    LOG(BREAK, cpu);
}

void cpu_execute_div(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];

    int32_t rs = (int32_t)cpu->r[RS];
    int32_t rt = (int32_t)cpu->r[RT];

    if(!cpu->r[RT]) //if rt is zero...
    {
            cpu->hi = cpu->r[RS];
            cpu->lo = (rs >= 0) ? 0xFFFFFFFF : 1;
    }
    else if((rs == 0x80000000) && cpu->r[RT] == -1)
    {
        cpu->hi = 0;
        cpu->lo = 0x80000000;
    }
    else
    {
        cpu->lo = (uint32_t)(rs / rt);
        cpu->hi = (uint32_t)(rs % rt);    
    }
    
    LOG(DIV, cpu);
}

void cpu_execute_divu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    //TODO: check if division by zero has to be handled in any way
    if(!cpu->r[RT])
    {
        cpu->lo = 0xffffffff;
        cpu->hi = cpu->r[RS];
    }
    else
    {
        cpu->lo = cpu->r[RS] / cpu->r[RT];
        cpu->hi = cpu->r[RS] % cpu->r[RT];
    }
    LOG(DIVU, cpu);
}

void cpu_execute_jalr(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    uint32_t target_address = cpu->r[RS];
    cpu->branch_address = target_address;
    cpu->branch = true;
    cpu->r[RD] = cpu->pc + 8;
    LOG(JALR, cpu);
}

void cpu_execute_jr(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    uint32_t target_address = cpu->r[RS];
    if((target_address & 0x3) != 0)
    {
        cpu_handle_exception(cpu, ADEL);
    }
    cpu->branch_address = target_address;
    cpu->branch = true;
    LOG(JR, cpu);
}

void cpu_execute_mfhi(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = cpu->hi;
    LOG(MFHI, cpu);
}

void cpu_execute_mflo(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = cpu->lo;
    LOG(MFLO, cpu);
}

void cpu_execute_mthi(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->hi = cpu->r[RS];
    LOG(MTHI, cpu);
}

void cpu_execute_mtlo(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->lo = cpu->r[RS];
    LOG(MTLO, cpu);
}

void cpu_execute_mult(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int64_t result = (int64_t)(int32_t)cpu->r[RS] * (int64_t)(int32_t)cpu->r[RT];
    cpu->lo = result & 0xFFFFFFFF;
    cpu->hi = (result >> 32) & 0xFFFFFFFF;
    LOG(MULT, cpu);
}

void cpu_execute_multu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    uint64_t result = (uint64_t)cpu->r[RS] * (uint64_t)cpu->r[RT];
    cpu->lo = result & 0xFFFFFFFF;
    cpu->hi = (result >> 32) & 0xFFFFFFFF;
    LOG(MULTU, cpu);
}

void cpu_execute_nor(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = ~(cpu->r[RS] | cpu->r[RT]);
    LOG(NOR, cpu);
}

void cpu_execute_or(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = cpu->r[RS] | cpu->r[RT];
    LOG(OR, cpu);
}

void cpu_execute_sll(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT]; //Extracting sa bits from opcode
    cpu->r[RD] = cpu->r[RT] << IMM5BITS;
    LOG(SLL, cpu);
}

void cpu_execute_sllv(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = cpu->r[RT] << (cpu->r[RS] & 0x1F);
    LOG(SLLV, cpu);
}

void cpu_execute_slt(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = ((int32_t)cpu->r[RS] < (int32_t)cpu->r[RT]) ? 1 : 0;
    LOG(SLT, cpu);
}

void cpu_execute_sltu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = (cpu->r[RS] < cpu->r[RT]) ? 1 : 0;
    LOG(SLTU, cpu);
}

void cpu_execute_sra(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = ((int32_t)cpu->r[RT] >> ((cpu->opcode >> 6) & 0x1F));
    LOG(SRA, cpu);
}

void cpu_execute_srav(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = ((int32_t)cpu->r[RT] >> (cpu->r[RS] & 0x1F));
    LOG(SRAV, cpu);
}

void cpu_execute_srl(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = (cpu->r[RT] >> ((cpu->opcode >> 6) & 0x1F)); 
    LOG(SRL, cpu);
}

void cpu_execute_srlv(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = (cpu->r[RT] >> (cpu->r[RS] & 0x1F));
    LOG(SRLV, cpu);
}

void cpu_execute_sub(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t value1 = cpu->r[RS];
    int32_t value2 = cpu->r[RT];
    int32_t result = value1 - value2;
    bool overflow = (((value1 ^ value2) & (value1 ^ result)) >> 31) & 0x1;
    if(overflow)
        cpu_handle_exception(cpu, OVERFLOW);  
    else
        cpu->r[RD] = result;

    LOG(SUB, cpu);

}

void cpu_execute_subu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = cpu->r[RS] - cpu->r[RT];
    LOG(SUBU, cpu);
}

void cpu_execute_syscall(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu_handle_exception(cpu, SYSCALL);
    LOG(INST_SYSCALL, cpu);
}

void cpu_execute_xor(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RD] = cpu->r[RS] ^ cpu->r[RT];
    LOG(XOR, cpu);
}

void cpu_execute_bgtz(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(!(cpu->r[RS] & 0x80000000) && (cpu->r[RS] != 0))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    LOG(BGTZ, cpu);
}

void cpu_execute_blez(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if((cpu->r[RS] & 0x80000000) || (cpu->r[RS] == 0))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    LOG(BLEZ, cpu);
}

void cpu_execute_bne(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(cpu->r[RS] != cpu->r[RT])
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    LOG(BNE, cpu);
}

void cpu_execute_jump(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->branch_address = (MASK26BITS << 2) | ((cpu->pc + 4) & 0xF0000000);
    cpu->branch = true;
    LOG(JUMP, cpu);
}

void cpu_execute_jal(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->branch_address = (MASK26BITS << 2) | ((cpu->pc + 4) & 0xF0000000);
    cpu->branch = true;
    cpu->r[31] = cpu->pc + 8;
    LOG(JAL, cpu);
}

void cpu_execute_lb(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];

    cpu->fifo_delay_load[0].delayed_value = (int32_t)(int8_t)ps1_bus_read_byte(cpu->bus, cpu->virtual_address);
    cpu->fifo_delay_load[0].delayed_register = RT;
    cpu->fifo_delay_load[0].modified = 1;
    cpu->fifo_delay_load[0].pc = cpu->pc;
    LOG(LB, cpu);
}   

void cpu_execute_lbu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];

    cpu->fifo_delay_load[0].delayed_value = ps1_bus_read_byte(cpu->bus, cpu->virtual_address);
    cpu->fifo_delay_load[0].delayed_register = RT;
    cpu->fifo_delay_load[0].modified = 1;
    cpu->fifo_delay_load[0].pc = cpu->pc;
    LOG(LBU, cpu);
}

void cpu_execute_lh(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];
    if(cpu->virtual_address & 0x1)
        cpu_handle_exception(cpu, ADEL);
    else
    {
        cpu->fifo_delay_load[0].delayed_value = (int32_t)(int16_t)ps1_bus_read_halfword(cpu->bus, cpu->virtual_address);
        cpu->fifo_delay_load[0].delayed_register = RT;
        cpu->fifo_delay_load[0].modified = 1;
        cpu->fifo_delay_load[0].pc = cpu->pc;
    }
    LOG(LH, cpu);
}

void cpu_execute_lhu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];
    if(cpu->virtual_address & 0x1)
        cpu_handle_exception(cpu, ADEL);
    else
    {
        cpu->fifo_delay_load[0].delayed_value = ps1_bus_read_halfword(cpu->bus, cpu->virtual_address);
        cpu->fifo_delay_load[0].delayed_register = RT;
        cpu->fifo_delay_load[0].modified = 1;
        cpu->fifo_delay_load[0].pc = cpu->pc;
    }
    LOG(LHU, cpu);
}

void cpu_execute_lui(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RT] = (uint32_t)IMM16BITS << 16;
    LOG(LUI, cpu);
}

void cpu_execute_lw(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];
    if(cpu->virtual_address & 0x3)
        cpu_handle_exception(cpu, ADEL);
    else
    {
        cpu->fifo_delay_load[0].delayed_value = ps1_bus_read_word(cpu->bus, cpu->virtual_address);
        cpu->fifo_delay_load[0].delayed_register = RT;
        cpu->fifo_delay_load[0].modified = 1;
        cpu->fifo_delay_load[0].pc = cpu->pc;
    }
    LOG(LW, cpu);
}

void cpu_execute_lwl(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];

    uint32_t word = ps1_bus_read_word(cpu->bus, cpu->virtual_address & 0xFFFFFFFC);  //Get the full word to then extract the necessary bytes
    uint8_t shift = ((cpu->virtual_address & 0x3) << 3);
    uint32_t mask = 0x00FFFFFF >> shift;

    uint32_t value = (word << (24 - shift)) | (cpu->r[RT] & mask);
    cpu->fifo_delay_load[0].delayed_value = value;
    cpu->fifo_delay_load[0].delayed_register = RT;
    cpu->fifo_delay_load[0].modified = 1;
    cpu->fifo_delay_load[0].pc = cpu->pc;

    LOG(LWL, cpu);
}

void cpu_execute_lwr(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];

    uint32_t word = ps1_bus_read_word(cpu->bus, cpu->virtual_address & 0xFFFFFFFC);  //Get the full word to then extract the necessary bytes
    uint8_t shift = ((cpu->virtual_address & 0x3) << 3);
    uint32_t mask = 0xFFFFFF00 << (24 - shift);

    uint32_t value = (word >> shift) | (cpu->r[RT] & mask);
    
    cpu->fifo_delay_load[0].delayed_value = value;
    cpu->fifo_delay_load[0].delayed_register = RT;
    cpu->fifo_delay_load[0].modified = 1;
    cpu->fifo_delay_load[0].pc = cpu->pc;

    LOG(LWR, cpu);
}

void cpu_execute_ori(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RT] = ((uint32_t)IMM16BITS | cpu->r[RS]);
    LOG(ORI, cpu);
}

void cpu_execute_sb(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    ps1_bus_store_byte(cpu->bus, cpu->virtual_address, (cpu->r[RT] & 0xFF));
    LOG(SB, cpu);
}

void cpu_execute_sh(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];

    if(cpu->virtual_address & 0x1)
        cpu_handle_exception(cpu, ADES);
    else  
        ps1_bus_store_halfword(cpu->bus, cpu->virtual_address, (cpu->r[RT] & 0xFFFF));
    LOG(SH, cpu);
}

void cpu_execute_slti(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RT] = ((int32_t)cpu->r[RS] < ((int32_t)(int16_t)IMM16BITS)) ? 1 : 0;
    LOG(SLTI, cpu);
}


void cpu_execute_sltiu(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RT] = (cpu->r[RS] < ((int32_t)(int16_t)IMM16BITS)) ? 1 : 0;
    LOG(SLTIU, cpu);
}

void cpu_execute_sw(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->virtual_address = (int32_t)(int16_t)IMM16BITS + cpu->r[BASE];
    if(cpu->virtual_address & 0x3)
        cpu_handle_exception(cpu, ADES);
    else
        ps1_bus_store_word(cpu->bus, cpu->virtual_address, cpu->r[RT]);
    LOG(SW, cpu);
}

void cpu_execute_swl(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];
    uint32_t word = ps1_bus_read_word(cpu->bus, cpu->virtual_address & 0xFFFFFFFC);
    
    uint8_t shift = ((cpu->virtual_address & 0x3) << 3);
    uint32_t mask = 0xFFFFFF00 << shift;
    uint32_t new_word = (word & mask) | (cpu->r[RT] >> (24 - shift));
    ps1_bus_store_word (cpu->bus, cpu->virtual_address & 0xFFFFFFFC, new_word);
    LOG(SWL, cpu);
}

void cpu_execute_swr(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = (int16_t)OFFSET16BITS;
    cpu->virtual_address = offset + cpu->r[BASE];
    uint32_t word = ps1_bus_read_word(cpu->bus, cpu->virtual_address & 0xFFFFFFFC);
    
    uint8_t shift = ((cpu->virtual_address & 0x3) << 3);
    uint32_t mask = 0x00FFFFFF >> (24 - shift);
    uint32_t new_word = (word & mask) | (cpu->r[RT] << shift);
    ps1_bus_store_word (cpu->bus, cpu->virtual_address & 0xFFFFFFFC, new_word);
    LOG(SWR, cpu);
}

void cpu_execute_xori(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RT] = cpu->r[RS] ^ IMM16BITS;
    LOG(XORI, cpu);
}

void cpu_execute_bgez(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(!(cpu->r[RS] & 0x80000000))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    LOG(BGEZ, cpu);
}

void cpu_execute_bgezal(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(!(cpu->r[RS] & 0x80000000))
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    cpu->r[31] = cpu->pc + 8; //Address of the instruction after the delay slot
    LOG(BGEZAL, cpu);
}

void cpu_execute_bltz(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(cpu->r[RS] & 0x80000000)
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }
    LOG(BLTZ, cpu);
}

void cpu_execute_bltzal(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    int32_t offset = ((int16_t)OFFSET16BITS) << 2; // offset, shifted left two bits and sign-extended.
    uint32_t target_address = offset + (cpu->pc + 4);
    if(cpu->r[RS] & 0x80000000)
    {
        cpu->branch_address = target_address;
        cpu->branch = true;
    }

    cpu->r[31] = cpu->pc + 8; //Address of the instruction after the delay slot
    LOG(BLTZAL, cpu);
}

void cpu_execute_mfc0(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->r[RT] = cpu->cop0[RD];
    LOG(MFC0, cpu);
}

void cpu_execute_mtc0(ps1_cpu* cpu)
{
    
    cpu->debug_rs_value = cpu->r[RS];
    cpu->debug_rt_value = cpu->r[RT];
    cpu->cop0[RD] = cpu->r[RT] & cpu_cop0_writemask[RD];
    LOG(MTC0, cpu);
}