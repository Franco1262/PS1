#include "bus.h"
#include "gpu.h"

ps1_gpu* ps1_gpu_create()
{
    return (ps1_gpu*)malloc(sizeof(ps1_gpu));
}

void ps1_gpu_init(ps1_gpu* gpu)
{
    memset(gpu, 0, sizeof(ps1_gpu));
}

void ps1_gpu_destroy(ps1_gpu* gpu)
{
    free (gpu);
}

uint32_t ps1_gpu_read_word(ps1_gpu* gpu, uint32_t address)
{
    uint32_t data = 0;

    switch(address)
    {
        case 0x1F801814:
            data = gpu->GPUSTAT;
            break;
    }

    return data;
}

void ps1_gpu_write_word(ps1_gpu* gpu, uint32_t address, uint32_t value)
{
    switch(address)
    {
        case 0x1F801810:
            printf("Command: %08x\n", value);
            break;
        case 0x1F801814:
            gpu->GPUSTAT = value;
            break;
    }   
}