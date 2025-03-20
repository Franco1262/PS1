#ifndef BUS_H
#define BUS_H

#include "stdint.h"
#include "stdlib.h"

typedef struct ps1_bios ps1_bios;
typedef struct ps1_cpu ps1_cpu;
typedef struct ps1_ram ps1_ram;

typedef struct ps1_bus
{
    ps1_bios* bios;
    ps1_cpu* cpu;
    ps1_ram* ram;
}ps1_bus;

ps1_bus* ps1_bus_create();
void ps1_bus_init(ps1_bus* bus, ps1_bios* bios, ps1_cpu* cpu, ps1_ram* ram);
uint8_t ps1_bus_read_byte(ps1_bus* bus, uint32_t address);
uint16_t ps1_bus_read_halfword(ps1_bus* bus, uint32_t address);
uint32_t ps1_bus_read_word(ps1_bus* bus, uint32_t address);

void ps1_bus_store_byte(ps1_bus* bus, uint32_t address, uint8_t value);
void ps1_bus_store_halfword(ps1_bus* bus, uint32_t address, uint16_t value);
void ps1_bus_store_word(ps1_bus* bus, uint32_t address, uint32_t value);
void ps1_bus_destroy(ps1_bus* bus);

#endif