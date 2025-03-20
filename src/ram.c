#include "ram.h"

ps1_ram* ps1_ram_create()
{
    return (ps1_ram*)malloc(sizeof(ps1_ram));
}

void ps1_ram_init(ps1_ram* ram)
{
    ram->ram_buff = malloc(RAM_SIZE);
}

void ps1_ram_destroy(ps1_ram* ram)
{
    if(ram->ram_buff != NULL)
        free (ram->ram_buff);
    free (ram);
}

uint32_t ps1_ram_read_word(ps1_ram* ram, uint32_t addr)
{
    return *(uint32_t*)(ram->ram_buff + (addr & (RAM_SIZE-1)));
}

void ps1_ram_store_word(ps1_ram* ram, uint32_t addr, uint32_t value)
{
    *(uint32_t*)(ram->ram_buff + (addr & (RAM_SIZE-1))) = value;
}

uint16_t ps1_ram_read_halfword(ps1_ram* ram, uint32_t addr)
{
    return *(uint16_t*)(ram->ram_buff + (addr & (RAM_SIZE-1)));
}

void ps1_ram_store_halfword(ps1_ram* ram, uint32_t addr, uint16_t value)
{
    *(uint16_t*)(ram->ram_buff + (addr & (RAM_SIZE-1))) = value;
}

uint8_t ps1_ram_read_byte(ps1_ram* ram, uint32_t addr)
{
    return *(ram->ram_buff + (addr & (RAM_SIZE-1)));
}

void ps1_ram_store_byte(ps1_ram* ram, uint32_t addr, uint8_t value)
{
    *(ram->ram_buff + (addr & (RAM_SIZE-1))) = value; 
}