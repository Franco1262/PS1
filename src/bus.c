#include "bus.h"
#include "cpu.h"
#include "ram.h"
#include "bios.h"

ps1_bus* ps1_bus_create()
{
    return (ps1_bus*)malloc(sizeof(ps1_bus));
}

void ps1_bus_init(ps1_bus* bus, ps1_bios* bios, ps1_cpu* cpu, ps1_ram* ram)
{
    bus->bios = bios;
    bus->cpu = cpu;
    bus->ram = ram;
}

uint8_t ps1_bus_read_byte(ps1_bus* bus, uint32_t address)
{
    return 0;
}

uint16_t ps1_bus_read_halfword(ps1_bus* bus, uint32_t address)
{
    return 0;
}

uint32_t ps1_bus_read_word(ps1_bus* bus, uint32_t address)
{
    return 0;
}

void ps1_bus_store_byte(ps1_bus* bus, uint32_t address, uint8_t value)
{

}

void ps1_bus_store_halfword(ps1_bus* bus, uint32_t address, uint16_t value)
{

}

void ps1_bus_store_word(ps1_bus* bus, uint32_t address, uint32_t value)
{

}

void ps1_bus_destroy(ps1_bus* bus)
{

}