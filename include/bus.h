#ifndef BUS_H
#define BUS_H

#include "stdint.h"

typedef struct ps1_bus
{
    //...
}ps1_bus;

uint8_t ps1_bus_read_byte(ps1_bus* bus, uint32_t address);
uint16_t ps1_bus_read_halfword(ps1_bus* bus, uint32_t address);
uint32_t ps1_bus_read_word(ps1_bus* bus, uint32_t address);

void ps1_bus_store_byte(ps1_bus* bus, uint32_t address, uint8_t value);
void ps1_bus_store_halfword(ps1_bus* bus, uint32_t address, uint16_t value);
void ps1_bus_store_word(ps1_bus* bus, uint32_t address, uint32_t value);

#endif