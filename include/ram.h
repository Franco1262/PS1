#ifndef RAM_H
#define RAM_H

#include "stdint.h"
#include "stdlib.h"


#define RAM_SIZE 0x200000

typedef struct ps1_ram
{
    uint8_t* ram_buff;
}ps1_ram;

ps1_ram* ps1_ram_create();
void ps1_ram_init(ps1_ram* ram);
void ps1_ram_destroy(ps1_ram* ram);

uint32_t ps1_ram_read(ps1_ram* ram, uint32_t addr);
void ps1_ram_write(ps1_ram* ram, uint32_t addr, uint32_t value);

#endif