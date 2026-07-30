#include "_abi.h"
#include "cartridge.h"

#include <stdlib.h>
#include <string.h>

static uint8_t *rom;
static uint8_t ram_extern[8 * KiB];

static uint8_t read_rom (memaddr address) {
    return rom[address];
}
static void write_rom(memaddr, uint8_t) {}
static bus_interface_t bus_rom =  { .read = read_rom, .write = write_rom };

static uint8_t read_ram_extern (memaddr address) {
    return ram_extern[address - ADDR_START_WRAM_CARTRIDGE];
}
static void write_ram_extern(memaddr address, uint8_t val) { 
    ram_extern[address - ADDR_START_WRAM_CARTRIDGE] = val;
}
static bus_interface_t bus_ram_extern =  { .read = read_ram_extern, .write = write_ram_extern };

void
init_cartridge(gb_bus_t *bus)
{
    bus->interface_rom_fixed = &bus_rom;
    bus->interface_rom_bank = &bus_rom;
    bus->interface_wram_extern = &bus_ram_extern;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions
        
###############################################################################
############################################################################ */

gb_return_t
GB_load_rom(const uint8_t *data, size_t size)
{
    if (size < 32 * KiB)
        return GB_ERROR_ROM_SIZE_MINIMUM;
    rom = (uint8_t *) malloc(size * sizeof(uint8_t));
    if (!rom)
        return GB_ERROR_MALLOC;
    memcpy(rom, data, size);
}