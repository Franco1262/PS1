#include "cpu.h"
#include "ram.h"
#include "bios.h"
#include "gpu.h"
#include "bus.h"

ps1_bus* ps1_bus_create()
{
    return (ps1_bus*)malloc(sizeof(ps1_bus));
}

void ps1_bus_init(ps1_bus* bus, ps1_bios* bios, ps1_cpu* cpu, ps1_ram* ram, ps1_gpu* gpu)
{
    bus->bios = bios;
    bus->cpu = cpu;
    bus->ram = ram;
    bus->gpu = gpu;
}

uint8_t ps1_bus_read_byte(ps1_bus* bus, uint32_t address)
{
    uint8_t data = 0x00;
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space

    if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
        data = ps1_ram_read_byte(bus->ram, masked_address);

    else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000) // BIOS ROM (512KB, max 4MB)
        data = ps1_bios_read_byte(bus->bios, masked_address);
/*     else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
        printf("Unhandled memory read at 0x%08X, tried to read byte from scratchpad  PC: %08x\n", address, bus->cpu->pc); */
/*     else if(masked_address >= 0x1F000000 && masked_address < 0x1F800000)
        printf("Unhandled memory read at 0x%08X, tried to read word from Expansion Region 1  PC: %08x\n", address, bus->cpu->pc);



    else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
        printf("Unhandled memory read at 0x%08X, tried to read byte from IO ports  PC: %08x\n", address, bus->cpu->pc);

    else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
        printf("Unhandled memory read at 0x%08X, tried to read byte from Expansion Region 2  PC: %08x\n", address, bus->cpu->pc);

    else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
        printf("Unhandled memory read at 0x%08X, tried to read byte from Expansion Region 3  PC: %08x\n", address, bus->cpu->pc);

    else if (address >= 0xFFFE0000 && address < 0xFFFE0200) // Cache Control Registers
        printf("Unhandled memory read at 0x%08X, tried to read byte from cache control registers  PC: %08x\n", address, bus->cpu->pc);

    else 
        printf("Unhandled byte memory read at 0x%08X, address falls in an unknown region  PC: %08x\n", address, bus->cpu->pc); */

    return data;
}

uint16_t ps1_bus_read_halfword(ps1_bus* bus, uint32_t address)
{
    uint16_t data = 0x00;
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space

    if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
        data = ps1_ram_read_halfword(bus->ram, masked_address);
    else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000) // BIOS ROM (512KB, max 4MB)
        data = ps1_bios_read_halfword(bus->bios, masked_address);
/*     else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
        printf("Unhandled memory read at 0x%08X, tried to read halfword from scratchpad  PC: %08x\n", address, bus->cpu->pc); */
/*     else if(masked_address >= 0x1F000000 && masked_address < 0x1F800000)
        printf("Unhandled memory read at 0x%08X, tried to read halfword from Expansion Region 1  PC: %08x\n", address, bus->cpu->pc);



    else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
        printf("Unhandled memory read at 0x%08X, tried to read halfword from IO ports  PC: %08x\n", address, bus->cpu->pc);

    else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
        printf("Unhandled memory read at 0x%08X, tried to read halfword from Expansion Region 2  PC: %08x\n", address, bus->cpu->pc);

    else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
        printf("Unhandled memory read at 0x%08X, tried to read halfword from Expansion Region 3  PC: %08x\n", address, bus->cpu->pc);

    else if (address >= 0xFFFE0000 && address < 0xFFFE0200) // Cache Control Registers
        printf("Unhandled memory read at 0x%08X, tried to read halfword from cache control registers  PC: %08x\n", address, bus->cpu->pc);

    else 
        printf("Unhandled halfword memory read at 0x%08X, address falls in an unknown region  PC: %08x\n", address, bus->cpu->pc); */

    return data;
}

uint32_t ps1_bus_read_word(ps1_bus* bus, uint32_t address)
{
    uint32_t data = 0xFFFFFFFF;
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space

    if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
        data = ps1_ram_read_word(bus->ram, masked_address);
    else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000) // BIOS ROM (512KB, max 4MB)
        data = ps1_bios_read_word(bus->bios, masked_address);
/*     else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
        printf("Unhandled memory read at 0x%08X, tried to read word from scratchpad  PC: %08x\n", address, bus->cpu->pc); */

/*     else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
        printf("Unhandled memory read at 0x%08X, tried to read word from I/O Ports  PC: %08x\n", address, bus->cpu->pc);
    else if(masked_address >= 0x1F000000 && masked_address < 0x1F800000)
        printf("Unhandled memory read at 0x%08X, tried to read word from Expansion Region 1  PC: %08x\n", address, bus->cpu->pc);

    else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
        printf("Unhandled memory read at 0x%08X, tried to read word from Expansion Region 2  PC: %08x\n", address, bus->cpu->pc);

    else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
        printf("Unhandled memory read at 0x%08X, tried to read word from Expansion Region 3  PC: %08x\n", address, bus->cpu->pc);

    else if (address >= 0xFFFE0000 && address < 0xFFFE0200) // Cache Control Registers
        printf("Unhandled memory read at 0x%08X, tried to read word from cache control registers  PC: %08x\n", address, bus->cpu->pc);

    else 
        printf("Unhandled word memory read at 0x%08X, address falls in an unknown region  PC: %08x\n", address, bus->cpu->pc); */

    return data;
}


void ps1_bus_store_byte(ps1_bus* bus, uint32_t address, uint8_t value)
{
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space
    if (!(bus->cpu->cop0[COP0_SR] & 0x10000))
    {
        if (address < 0xFFFE0000)  // Ignore CPU control registers
        {
            if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
                ps1_ram_store_byte(bus->ram, masked_address, value);      
/*             else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
                printf("Unhandled memory write at 0x%08X, tried to write byte to scratchpad  PC: %08x\n", address, bus->cpu->pc); */
/*             else if(masked_address >= 0x1F000000 && masked_address < 0x1F800000)
                printf("Unhandled memory write at 0x%08X, tried to write byte to Expansion Region 1  PC: %08x\n", address, bus->cpu->pc);


            
            else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
                printf("Unhandled memory write at 0x%08X, tried to write byte to IO ports  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
                printf("Unhandled memory write at 0x%08X, tried to write byte to Expansion Region 2  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
                printf("Unhandled memory write at 0x%08X, tried to write byte to Expansion Region 3  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000)  // BIOS ROM (512KB, max 4MB)
                printf("Unhandled memory write at 0x%08X, tried to write byte to BIOS  PC: %08x\n", address, bus->cpu->pc);
            
            else
                printf("Unhandled byte memory write at 0x%08X, address falls in an unknown region\n", address); */
        }
/*         else if (address >= 0xFFFE0000 && address < 0xFFFE0200)
            printf("Unhandled memory write at 0x%08X, tried to write byte to cache control registers\n", address);
        else
            printf("Unhandled byte memory write at 0x%08X, address falls in an unknown region\n", address); */
    }
}

void ps1_bus_store_halfword(ps1_bus* bus, uint32_t address, uint16_t value)
{
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space
    if (!(bus->cpu->cop0[COP0_SR] & 0x10000))
    {
        if (address < 0xFFFE0000)  // Ignore CPU control registers
        {
            if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
                ps1_ram_store_halfword(bus->ram, masked_address, value);
/*             else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to scratchpad  PC: %08x\n", address, bus->cpu->pc); */
/*             else if(masked_address >= 0x1F000000 && masked_address < 0x1F800000)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to Expansion Region 1  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to IO ports  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to Expansion Region 2  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to Expansion Region 3  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000)  // BIOS ROM (512KB, max 4MB)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to BIOS  PC: %08x\n", address, bus->cpu->pc);
            
            else
                printf("Unhandled halfword memory write at 0x%08X, address falls in an unknown region  PC: %08x\n", address, bus->cpu->pc); */
        }
/*         else if (address >= 0xFFFE0000 && address < 0xFFFE0200)
            printf("Unhandled memory write at 0x%08X, tried to write halfword to cache control registers  PC: %08x\n", address, bus->cpu->pc);
        else
            printf("Unhandled halfword memory write at 0x%08X, address falls in an unknown region  PC: %08x\n", address, bus->cpu->pc); */
    }
}


void ps1_bus_store_word(ps1_bus* bus, uint32_t address, uint32_t value)
{
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space

    if (!(bus->cpu->cop0[COP0_SR] & 0x10000))
    {
        if (address < 0xFFFE0000)  // Ignore CPU control registers
        {
            if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
                ps1_ram_store_word(bus->ram, masked_address, value);
/*             else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
                printf("Unhandled memory write at 0x%08X, tried to write word to scratchpad  PC: %08x\n", address, bus->cpu->pc); */

/*             else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
                printf("Unhandled memory write at 0x%08X, tried to write word to IO ports  PC: %08x\n", address, bus->cpu->pc);
            else if(masked_address >= 0x1F000000 && masked_address < 0x1F800000)
                printf("Unhandled memory write at 0x%08X, tried to write word to Expansion Region 1  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
                printf("Unhandled memory write at 0x%08X, tried to write word to Expansion Region 2  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
                printf("Unhandled memory write at 0x%08X, tried to write word to Expansion Region 3  PC: %08x\n", address, bus->cpu->pc);
            
            else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000)  // BIOS ROM (512KB, max 4MB)
                printf("Unhandled memory write at 0x%08X, tried to write word to BIOS  PC: %08x\n", address, bus->cpu->pc);
            
            else
                printf("Unhandled word memory write at 0x%08X, address falls in an unknown region\n", address); */
            
        }
/*     else if (address >= 0xFFFE0000 && address <= 0xFFFE0200)
        printf("Unhandled memory write at 0x%08X, tried to write word to cache control registers  PC: %08x\n", address, bus->cpu->pc);
    else
        printf("Unhandled word memory write at 0x%08X, address falls in an unknown region\n", address);  */
    }
}

ps1_ram* ps1_bus_get_ram(ps1_bus* bus)
{
    return bus->ram;
}

void ps1_bus_destroy(ps1_bus* bus)
{
    free (bus);
}