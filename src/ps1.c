#include "ram.h"
#include "bios.h"
#include "bus.h"
#include "cpu.h"
#include "gpu.h"
#include "scratchpad.h"
#include "dma.h"
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
    ps1->gpu = ps1_gpu_create();
    ps1->scratchpad = ps1_scratchpad_create();
    ps1->dma = ps1_dma_create();
  
    ps1_dma_init(ps1->dma);
    ps1_bios_init(ps1->bios);
    ps1_cpu_init(ps1->cpu);
    ps1_ram_init(ps1->ram);
    ps1_gpu_init(ps1->gpu);
    ps1_scratchpad_init(ps1->scratchpad);
    ps1_bus_init(ps1->bus, ps1->bios, ps1->cpu, ps1->ram, ps1->gpu, ps1->scratchpad, ps1->dma);

    ps1_connect_bus_cpu(ps1->bus, ps1->cpu);
    ps1_connect_bus_dma(ps1->bus, ps1->dma);
}

void ps1_destroy(ps1* ps1)
{
    free (ps1->bios);
    free (ps1->ram);
    free (ps1->cpu);
    free (ps1->bus);
    free (ps1->dma);
    free (ps1);
}

void ps1_load_bios(ps1* ps1)
{
    ps1_bios_load(ps1->bios);
}

void ps1_play(ps1* ps1)
{
    cpu_tick(ps1->cpu);
    ps1_dma_do_transfer(ps1->dma);
    
}