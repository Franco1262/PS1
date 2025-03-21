#include "bios.h"

ps1_bios* ps1_bios_create()
{
    return (ps1_bios*)malloc(sizeof(ps1_bios));
}

void ps1_bios_init(ps1_bios* bios)
{
    memset(bios, 0, sizeof(ps1_bios)); // Initialize bios struct to 0
    bios->buffer = NULL; // Initialize buffer to NULL
}

uint8_t ps1_bios_read_byte(ps1_bios* bios, uint32_t addr)
{
    uint32_t offset = addr & 0x003FFFFF;
    return *(bios->buffer + offset);
}

uint16_t ps1_bios_read_halfword(ps1_bios* bios, uint16_t addr)
{
    uint32_t offset = addr & 0x003FFFFF;
    return *(uint16_t*)(bios->buffer + offset);   
}

uint32_t ps1_bios_read_word(ps1_bios* bios, uint32_t addr)
{
    uint32_t offset = addr & 0x003FFFFF;
    return *(uint32_t*)(bios->buffer + offset);
}

void ps1_bios_load(ps1_bios* bios)
{
    // Ensure bios->buffer is allocated
    bios->buffer = (uint8_t*)malloc(BIOS_SIZE);
    if (bios->buffer == NULL) 
    {
        perror("Error: Could not allocate memory for BIOS buffer.");
        return;
    }

    FILE *file = fopen("SCPH1001.BIN", "rb");
    if (file == NULL) 
    {
        perror("Error: Could not open BIOS file.");
        free(bios->buffer); // Free the allocated buffer if the file open fails
        return;
    }

    uint32_t bytes_read = fread(bios->buffer, 1, BIOS_SIZE, file);
    if (bytes_read != BIOS_SIZE) 
    {
        perror("Error: Could not read the entire BIOS file.");
        fclose(file);
        free(bios->buffer); // Free the buffer if reading fails
        return;
    }

    fclose(file);
}

void ps1_bios_destroy(ps1_bios* bios)
{
    if (bios->buffer != NULL) {
        free(bios->buffer);  // Free the buffer if it was allocated
    }
    free(bios);  // Free the struct itself
}
