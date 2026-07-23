#include "bus.h"

#include <stdio.h>

static uint8_t boot_rom[GB_DMG_BOOT_ROM_SIZE];

static uint8_t read_boot_rom (memaddr address)  { return boot_rom[address & GB_DMG_BOOT_ROM_SIZE]; }
static void    write_boot_rom(memaddr, uint8_t) {}
static bus_interface_t bus_boot_rom =  { read_boot_rom, write_boot_rom };

int
init_bootrom(char *filename, gb_bus_t *bus)
{
    bus->interface_rom_boot = &bus_boot_rom;

    FILE *f = fopen(filename, "rb");
    if (f == NULL)
        return -1;
    size_t bytes_read = fread(boot_rom, 1, GB_DMG_BOOT_ROM_SIZE, f);
    fclose(f);
    if (bytes_read != GB_DMG_BOOT_ROM_SIZE)
        return -1;
    return 0;
}