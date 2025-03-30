#ifndef GPU_H
#define GPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct ps1_bus ps1_bus;
typedef struct ps1_gpu
{
    uint32_t GPUSTAT;
}ps1_gpu;

uint32_t ps1_gpu_read_word(ps1_gpu* gpu, uint32_t address);
void ps1_gpu_write_word(ps1_gpu* gpu, uint32_t address, uint32_t value);

ps1_gpu* ps1_gpu_create();
void ps1_gpu_init(ps1_gpu* gpu);
void ps1_gpu_destroy(ps1_gpu* gpu);

#endif