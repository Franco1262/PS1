#include "bus.h"
#include "dma.h"
#include "log.h"

ps1_dma* ps1_dma_create()
{
    return (ps1_dma*)malloc(sizeof(ps1_dma));
}
void ps1_dma_init(ps1_dma* dma)
{
    memset(dma, 0, sizeof(ps1_dma));
    dma->dpcr = 0x07654321;
}

void ps1_dma_destroy(ps1_dma* dma)
{
    free (dma);
}

void ps1_dma_store_word(ps1_dma* dma, uint32_t address, uint32_t value)
{
    uint8_t channel = (address >> 4) & 0x7;
    uint8_t reg = address & 0xF;
    switch (reg)
    {
        case 0x0:
            if(address == 0x1F8010F0)
                dma->dpcr = value;
            else
                dma->channel[channel].madr = value; 
            break;
        case 0x4: 
            if(address == 0x1F8010F4)
                dma->dicr = value;
            else
                dma->channel[channel].bcr = value; 
            break;
        case 0x8: dma->channel[channel].chcr = value; break;
        case 0xC: dma->channel[channel].chcr = value; break;
        default: printf("Unhandled word write to DMA address: %08x\n", address); break;;
    }
    printf("Write to %08x, value written: %08x\n", address, value);
}

//Even though it's a halfword write the whole 32bits of the bus are written to the register as explained in https://psx-spx.consoledev.net/unpredictablethings/
//same applies to byte writes
void ps1_dma_store_halfword(ps1_dma* dma, uint32_t address, uint16_t value)
{
    uint8_t channel = (address >> 4) & 0xF;
    uint8_t reg = address & 0xF;
    switch (reg)
    {
        case 0x0:
            if(address == 0x1F8010F0)
                dma->dpcr = value;
            else
                dma->channel[channel].madr = value; 
            break;
        case 0x4: 
            if(address == 0x1F8010F4)
                dma->dicr = value;
            else
                dma->channel[channel].bcr = value; 
            break;
        case 0x8: dma->channel[channel].chcr = value; break;
        case 0xC: dma->channel[channel].chcr = value; break;
        default: printf("Unhandled halfword write to DMA address: %08x", address); break;;
    }
}

void ps1_dma_store_byte(ps1_dma* dma, uint32_t address, uint8_t value)
{
    uint8_t channel = (address >> 4) & 0xF;
    uint8_t reg = address & 0xF;
    switch (reg)
    {
        case 0x0:
            if(address == 0x1F8010F0)
                dma->dpcr = value;
            else
                dma->channel[channel].madr = value; 
            break;
        case 0x4: 
            if(address == 0x1F8010F4)
                dma->dicr = value;
            else
                dma->channel[channel].bcr = value; 
            break;
        case 0x8: dma->channel[channel].chcr = value; break;
        case 0xC: dma->channel[channel].chcr = value; break;
        default: printf("Unhandled byte write to DMA address: %08x\n", address); break;;
    }
}

uint8_t ps1_dma_read_byte(ps1_dma* dma, uint32_t address)
{
    printf("Unhandled BYTE read from DMA address 0x%08X\n", address);
    return 0xFF;
}

uint16_t ps1_dma_read_halfword(ps1_dma* dma, uint32_t address)
{
    printf("Unhandled HALFWORD read from DMA address 0x%08X\n", address);
    return 0xFFFF;
}

uint32_t ps1_dma_read_word(ps1_dma* dma, uint32_t address)
{
    uint32_t data = 0x0;
    uint8_t channel = (address >> 4) & 0xF;
    uint8_t reg = address & 0xF;
    switch (reg)
    {
        case 0x0:
            if(address == 0x1F8010F0)
                data = dma->dpcr;
            else
                data = dma->channel[channel].madr; 
            break;
        case 0x4: 
            if(address == 0x1F8010F4)
                data = dma->dicr;
            else
                data = dma->channel[channel].bcr; 
            break;
        case 0x8: data = dma->channel[channel].chcr; break;
        case 0xC: data = dma->channel[channel].chcr; break;
        default: printf("Unhandled word read to DMA address: %08x\n", address); break;;
    }

    printf("Read from %08x, value read: %08x\n", address, data);
    return data;
}

void ps1_dma_do_transfer(ps1_dma* dma)
{
    if(dma->channel[6].chcr == 0x11000002) //Clear OTC
    {
        printf("START OTC\n");
        ps1_dma_do_otc(dma);       
    }

    else if(dma->channel[2].chcr == 0x01000401) //GPU linked list
    {
        printf("START GPU LINKLIST\n");
        ps1_dma_do_linklist(dma);
    }
    
}

void ps1_dma_do_linklist(ps1_dma* dma)
{
    uint16_t num_packets = (dma->channel[2].madr >> 24) & 0xFF;
    uint32_t addr = dma->channel[2].madr & 0x00FFFFFF;
    for(int i = 0; i < num_packets ; i++)
    {

    }

    printf("FINISH GPU LINKLIST\n");
    dma->channel[2].chcr = ~( (1 << 24) | (1 << 28) );
}

void ps1_dma_do_otc(ps1_dma* dma)
{
    //TODO: What happens when chopping enabled?
    int8_t increment_type = (dma->channel[6].chcr & 0x2 ? -4 : 4);
    int32_t increment = 0;
    uint32_t address = 0;

    for(int i = 0; i < dma->channel[6].bcr; i++)
    {
        address = dma->channel[6].madr + increment;
        increment += increment_type;
        if(i == (dma->channel[6].bcr - 1))
            ps1_bus_store_word(dma->bus, address, 0x00FFFFFF);

        else
            ps1_bus_store_word(dma->bus, address, (0x00000000 | (0x00FFFFFF & (address + increment_type))) );

        
    }
    dma->channel[6].chcr = ~( (1 << 24) | (1 << 28) );
    printf("FINISH OTC\n");
}

void ps1_connect_bus_dma(ps1_bus* bus, ps1_dma* dma)
{
    dma->bus = bus;
}




