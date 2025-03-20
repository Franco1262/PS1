#ifndef BIOS_H
#define BIOS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BIOS_SIZE 512*1024

typedef struct ps1_bios
{
    uint8_t* buffer;
} ps1_bios;

ps1_bios* ps1_bios_create();
void ps1_bios_init(ps1_bios* bios);
uint8_t ps1_bios_read(ps1_bios* bios, uint32_t addr);
void ps1_bios_load(ps1_bios* bios);
void ps1_bios_destroy(ps1_bios* bios);

#endif