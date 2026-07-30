#include "_abi.h"
#include "cartridge.h"

#include <stdlib.h>
#include <string.h>

static gb_cartridge_t cartridge;
static uint8_t ram_extern[8 * KiB];

static uint8_t read_rom (memaddr address) {
    return cartridge.rom[address];
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

static size_t
reported_header_rom_size(void)
{
    switch (cartridge.header.rom_size) {
    case MBC_ROM_SIZE_32KiB:                    return   32 * KiB;
    case MBC_ROM_SIZE_64KiB:                    return   64 * KiB;
    case MBC_ROM_SIZE_128KiB:                   return  128 * KiB;
    case MBC_ROM_SIZE_256KiB:                   return  256 * KiB;
    case MBC_ROM_SIZE_512KiB:                   return  512 * KiB;
    case MBC_ROM_SIZE_1MiB:                     return    1 * MiB;
    case MBC_ROM_SIZE_2MiB:                     return    2 * MiB;
    case MBC_ROM_SIZE_4MiB:                     return    4 * MiB;
    case MBC_ROM_SIZE_8MiB:                     return    8 * MiB;
    case MBC_ROM_SIZE_1152KiB:                  return 1152 * KiB;
    case MBC_ROM_SIZE_1280KiB:                  return 1280 * KiB;
    case MBC_ROM_SIZE_1536KiB:                  return 1536 * KiB;
    default:                                    return    0      ;
    }
}

gb_cartridge_t *
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
    cartridge.rom = (uint8_t *) malloc(size * sizeof(uint8_t));
    if (!cartridge.rom)
        return GB_ERROR_MALLOC;
    memcpy(cartridge.rom, data, size);

    // read header
    cartridge.header.mbc_type = cartridge.rom[CARTRIDGE_HEADER_MBC_TYPE];
    cartridge.header.rom_size = cartridge.rom[CARTRIDGE_HEADER_ROM_SIZE];
    cartridge.header.ram_size = cartridge.rom[CARTRIDGE_HEADER_RAM_SIZE];

    if (reported_header_rom_size() != size)
        return GB_ERROR_ROM_HEADER_SIZE;

    cartridge.rom_size_bytes = size;

    return GB_RETURN_OK;
}