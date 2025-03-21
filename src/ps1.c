#include "ram.h"
#include "bios.h"
#include "bus.h"
#include "cpu.h"
#include "ps1.h"

ps1* ps1_create()
{
    return (ps1*)malloc(sizeof(ps1));
}

void ps1_init(ps1* ps1)
{
    ps1->bios = ps1_bios_create();
    ps1->cpu = ps1_cpu_create();
    ps1->ram = ps1_ram_create();
    ps1->bus = ps1_bus_create();
  
    ps1_bios_init(ps1->bios);
    ps1_cpu_init(ps1->cpu);
    ps1_ram_init(ps1->ram);
    ps1_bus_init(ps1->bus, ps1->bios, ps1->cpu, ps1->ram);

    ps1_connect_bus(ps1->bus, ps1->cpu);
}

void ps1_destroy(ps1* ps1)
{
    free (ps1->bios);
    free (ps1->ram);
    free (ps1->cpu);
    free (ps1->bus);
    free (ps1);
}

void ps1_load_bios(ps1* ps1)
{
    ps1_bios_load(ps1->bios);
}
void ps1_play(ps1* ps1)
{
    cpu_tick(ps1->cpu);
}