#ifndef PS1_H
#define PS1_H

#include <stdint.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>

typedef struct ps1_cpu ps1_cpu;
typedef struct ps1_ram ps1_ram;
typedef struct ps1_bios ps1_bios;
typedef struct ps1_bus ps1_bus;
typedef struct ps1_gpu ps1_gpu;
typedef struct ps1_scratchpad ps1_scratchpad;

typedef struct ps1
{
    ps1_cpu* cpu;
    ps1_bus* bus;
    ps1_ram* ram;
    ps1_bios* bios;
    ps1_gpu* gpu;
    ps1_scratchpad* scratchpad;

}ps1;

ps1* ps1_create();
void ps1_init(ps1* ps1);
void ps1_destroy(ps1* ps1);
void ps1_load_bios(ps1* ps1);
void ps1_play(ps1* ps1);

#endif