#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum DMA_REGISTERS 
{
    DMA_MDEC_IN      = 0x1F801080, // DMA0 channel 0 - MDECin
    DMA_MDEC_OUT     = 0x1F801090, // DMA1 channel 1 - MDECout
    DMA_GPU          = 0x1F8010A0, // DMA2 channel 2 - GPU (lists + image data)
    DMA_CDROM        = 0x1F8010B0, // DMA3 channel 3 - CDROM
    DMA_SPU          = 0x1F8010C0, // DMA4 channel 4 - SPU
    DMA_PIO          = 0x1F8010D0, // DMA5 channel 5 - PIO (Expansion Port)
    DMA_OTC          = 0x1F8010E0, // DMA6 channel 6 - OTC (reverse clear OT) (GPU related)
    DMA_DPCR         = 0x1F8010F0, // DPCR - DMA Control register
    DMA_DICR         = 0x1F8010F4  // DICR - DMA Interrupt register
} DMA_REGISTERS;


typedef struct channel
{
    uint32_t madr;
    uint32_t bcr;
    uint32_t chcr;
}channel;

typedef struct ps1_bus ps1_bus;

typedef struct ps1_dma
{   
    channel channel[7];
    uint32_t dpcr;
    uint32_t dicr;
    ps1_bus* bus;
}ps1_dma;


ps1_dma* ps1_dma_create();
void ps1_dma_init(ps1_dma* dma);
void ps1_dma_destroy(ps1_dma* dma);
void ps1_connect_bus_dma(ps1_bus* bus, ps1_dma* dma);
void ps1_dma_do_transfer(ps1_dma* dma);
void ps1_dma_do_otc(ps1_dma* dma);
void ps1_dma_do_linklist(ps1_dma* dma);

uint8_t ps1_dma_read_byte(ps1_dma* dma, uint32_t address);
uint16_t ps1_dma_read_halfword(ps1_dma* dma, uint32_t address);
uint32_t ps1_dma_read_word(ps1_dma* dma, uint32_t address);

void ps1_dma_store_byte(ps1_dma* dma, uint32_t address, uint8_t value);
void ps1_dma_store_halfword(ps1_dma* dma, uint32_t address, uint16_t value);
void ps1_dma_store_word(ps1_dma* dma, uint32_t address, uint32_t value);


#endif