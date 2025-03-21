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
uint8_t ps1_bios_read_byte(ps1_bios* bios, uint32_t addr);
uint16_t ps1_bios_read_halfword(ps1_bios* bios, uint16_t addr);
uint32_t ps1_bios_read_word(ps1_bios* bios, uint32_t addr);
void ps1_bios_load(ps1_bios* bios);
void ps1_bios_destroy(ps1_bios* bios);

#endif