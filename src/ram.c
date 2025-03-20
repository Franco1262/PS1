#include "ram.h"

ps1_ram* ps1_ram_create()
{
    return (ps1_ram*)malloc(sizeof(ps1_ram));
}

void ps1_ram_init(ps1_ram* ram)
{
    ram->ram_buff = (uint32_t*)malloc(RAM_SIZE);
}

void ps1_ram_destroy(ps1_ram* ram)
{
    if(ram->ram_buff != NULL)
        free (ram->ram_buff);
    free (ram);
}

uint32_t ps1_ram_read(ps1_ram* ram, uint32_t addr)
{
    return 0;
}

void ps1_ram_write(ps1_ram* ram, uint32_t addr, uint32_t value)
{

}