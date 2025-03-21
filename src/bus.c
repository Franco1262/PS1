#include "cpu.h"
#include "ram.h"
#include "bios.h"
#include "bus.h"

ps1_bus* ps1_bus_create()
{
    return (ps1_bus*)malloc(sizeof(ps1_bus));
}

void ps1_bus_init(ps1_bus* bus, ps1_bios* bios, ps1_cpu* cpu, ps1_ram* ram)
{
    bus->bios = bios;
    bus->cpu = cpu;
    bus->ram = ram;
}

uint8_t ps1_bus_read_byte(ps1_bus* bus, uint32_t address)
{
    uint8_t data = 0x00;
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space

    if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
        data = ps1_ram_read_byte(bus->ram, masked_address);

    else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000) // BIOS ROM (512KB, max 4MB)
    {
        data = ps1_bios_read_byte(bus->bios, masked_address);
    }

    else 
    {
        // Handle invalid access (e.g., return 0 or log an error)
        printf("Unhandled memory read at 0x%08X\n", address);
    }

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
    else 
    {
        // Handle invalid access (e.g., return 0 or log an error)
        printf("Unhandled memory read at 0x%08X\n", address);
    }

    return data;
}

uint32_t ps1_bus_read_word(ps1_bus* bus, uint32_t address)
{
    uint32_t data = 0x00;
    uint32_t masked_address = address & 0x1FFFFFFF; // Mask to 512MB space

    if (masked_address < 0x00200000)  // Main RAM (2MB, first 64K reserved for BIOS)
        data = ps1_ram_read_word(bus->ram, masked_address);

    else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000) // BIOS ROM (512KB, max 4MB)
        data = ps1_bios_read_word(bus->bios, masked_address);
    
    else 
    {
        // Handle invalid access (e.g., return 0 or log an error)
        printf("Unhandled memory read at 0x%08X\n", address);
    }

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
            
            else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
                printf("Unhandled memory write at 0x%08X, tried to write byte to scratchpad\n", address);
            
            else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
                printf("Unhandled memory write at 0x%08X, tried to write byte to IO ports\n", address);
            
            else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
                printf("Unhandled memory write at 0x%08X, tried to write byte to Expansion Region 2\n", address);
            
            else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
                printf("Unhandled memory write at 0x%08X, tried to write byte to Expansion Region 3\n", address);
            
            else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000)  // BIOS ROM (512KB, max 4MB)
                printf("Unhandled memory write at 0x%08X, tried to write byte to BIOS\n", address);
            
            else
                printf("Unhandled byte memory write at 0x%08X, address falls in an unknown region\n", address);
        }
        else if (address >= 0xFFFE0000 && address < 0xFFFE0200)
            printf("Unhandled memory write at 0x%08X, tried to write byte to cache control registers\n", address);
        else
            printf("Unhandled byte memory write at 0x%08X, address falls in an unknown region\n", address);
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
            
            else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to scratchpad\n", address);
            
            else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to IO ports\n", address);
            
            else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to Expansion Region 2\n", address);
            
            else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to Expansion Region 3\n", address);
            
            else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000)  // BIOS ROM (512KB, max 4MB)
                printf("Unhandled memory write at 0x%08X, tried to write halfword to BIOS\n", address);
            
            else
                printf("Unhandled halfword memory write at 0x%08X, address falls in an unknown region\n", address);
        }
        else if (address >= 0xFFFE0000 && address < 0xFFFE0200)
            printf("Unhandled memory write at 0x%08X, tried to write halfword to cache control registers\n", address);
        else
            printf("Unhandled halfword memory write at 0x%08X, address falls in an unknown region\n", address);
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
            
            else if (masked_address >= 0x1F800000 && masked_address < 0x1F801000)  // Scratchpad RAM (1KB)
                printf("Unhandled memory write at 0x%08X, tried to write word to scratchpad\n", address);
            
            else if (masked_address >= 0x1F801000 && masked_address < 0x1F802000)  // I/O Ports (4KB)
                printf("Unhandled memory write at 0x%08X, tried to write word to IO ports \n", address);
            
            else if (masked_address >= 0x1F802000 && masked_address < 0x1FA00000)  // Expansion Region 2 (8KB I/O)
                printf("Unhandled memory write at 0x%08X, tried to write word to Expansion Region 2\n", address);
            
            else if (masked_address >= 0x1FA00000 && masked_address < 0x1FC00000)  // Expansion Region 3 (DTL SRAM)
                printf("Unhandled memory write at 0x%08X, tried to write word to Expansion Region 3\n", address);
            
            else if (masked_address >= 0x1FC00000 && masked_address < 0x20000000)  // BIOS ROM (512KB, max 4MB)
                printf("Unhandled memory write at 0x%08X, tried to write word to BIOS\n", address);
            
            else
                printf("Unhandled word memory write at 0x%08X, address falls in an unknown region\n", address);
            
        }
        else if (address >= 0xFFFE0000 && address <= 0xFFFE0200)
            printf("Unhandled memory write at 0x%08X, tried to write word to cache control registers\n", address);
        else
            printf("Unhandled word memory write at 0x%08X, address falls in an unknown region\n", address); 
    }
}


void ps1_bus_destroy(ps1_bus* bus)
{
    free (bus);
}