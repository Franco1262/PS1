#include "scratchpad.h"

ps1_scratchpad* ps1_scratchpad_create()
{
    return (ps1_scratchpad*)malloc(sizeof(ps1_scratchpad));
}

void ps1_scratchpad_init(ps1_scratchpad* scratchpad)
{
    scratchpad->scratchpad_buff = malloc(SCRATCHPAD_SIZE);
    memset(scratchpad->scratchpad_buff, 0, SCRATCHPAD_SIZE);
}

void ps1_scratchpad_destroy(ps1_scratchpad* scratchpad)
{
    if(scratchpad->scratchpad_buff != NULL)
        free (scratchpad->scratchpad_buff);
    free (scratchpad);
}

uint32_t ps1_scratchpad_read_word(ps1_scratchpad* scratchpad, uint32_t addr)
{
    return *(uint32_t*)(scratchpad->scratchpad_buff + (addr & (SCRATCHPAD_SIZE-1)));
}

void ps1_scratchpad_store_word(ps1_scratchpad* scratchpad, uint32_t addr, uint32_t value)
{
    *(uint32_t*)(scratchpad->scratchpad_buff + (addr & (SCRATCHPAD_SIZE-1))) = value;
}

uint16_t ps1_scratchpad_read_halfword(ps1_scratchpad* scratchpad, uint32_t addr)
{
    return *(uint16_t*)(scratchpad->scratchpad_buff + (addr & (SCRATCHPAD_SIZE-1)));
}

void ps1_scratchpad_store_halfword(ps1_scratchpad* scratchpad, uint32_t addr, uint16_t value)
{
    *(uint16_t*)(scratchpad->scratchpad_buff + (addr & (SCRATCHPAD_SIZE-1))) = value;
}

uint8_t ps1_scratchpad_read_byte(ps1_scratchpad* scratchpad, uint32_t addr)
{
    return *(scratchpad->scratchpad_buff + (addr & (SCRATCHPAD_SIZE-1)));
}

void ps1_scratchpad_store_byte(ps1_scratchpad* scratchpad, uint32_t addr, uint8_t value)
{
    *(scratchpad->scratchpad_buff + (addr & (SCRATCHPAD_SIZE-1))) = value; 
}