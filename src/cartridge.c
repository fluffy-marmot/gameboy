#include "bus.h"
#include "cartridge.h"

#include <stdio.h>

static uint8_t rom[ROM_SIZE];
static uint8_t ram_extern[8 * KiB];

static uint8_t read_rom (memaddr address)  { return rom[address]; }
static void    write_rom(memaddr, uint8_t) {}
static bus_interface_t bus_rom =  { read_rom, write_rom };

static uint8_t read_ram_extern (memaddr address)             { return ram_extern[address - ADDR_STR_WRAMEXT]; }
static void    write_ram_extern(memaddr address, uint8_t val) { ram_extern[address - ADDR_STR_WRAMEXT] = val; }
static bus_interface_t bus_ram_extern =  { read_ram_extern, write_ram_extern };

int
init_cartridge(char *filename, gb_bus_t *bus)
{
    bus->interface_rom_fixed = &bus_rom;
    bus->interface_rom_bank = &bus_rom;
    bus->interface_wram_extern = &bus_ram_extern;

    FILE *f = fopen(filename, "rb");
    if (f == NULL)
        return -1;
    size_t bytes_read = fread(rom, 1, ROM_SIZE, f);
    fclose(f);
    if (bytes_read != ROM_SIZE)
        return -1;
    return 0;
}