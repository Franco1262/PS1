#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include "stdint.h"
#include <string.h>
#include <stdlib.h>


#define SCRATCHPAD_SIZE 0x400

typedef struct ps1_scratchpad
{
    uint8_t* scratchpad_buff;
}ps1_scratchpad;

ps1_scratchpad* ps1_scratchpad_create();
void ps1_scratchpad_init(ps1_scratchpad* scratchpad);
void ps1_scratchpad_destroy(ps1_scratchpad* scratchpad);

uint32_t ps1_scratchpad_read_word(ps1_scratchpad* scratchpad, uint32_t addr);
void ps1_scratchpad_store_word(ps1_scratchpad* scratchpad, uint32_t addr, uint32_t value);

uint16_t ps1_scratchpad_read_halfword(ps1_scratchpad* scratchpad, uint32_t addr);
void ps1_scratchpad_store_halfword(ps1_scratchpad* scratchpad, uint32_t addr, uint16_t value);

uint8_t ps1_scratchpad_read_byte(ps1_scratchpad* scratchpad, uint32_t addr);
void ps1_scratchpad_store_byte(ps1_scratchpad* scratchpad, uint32_t addr, uint8_t value);

#endif