#include "_abi.h"
#include "bootrom.h"

static uint8_t boot_rom[GB_DMG_BOOT_ROM_SIZE];

static uint8_t read_boot_rom (memaddr address) {
    return boot_rom[address & ADDR_END_ROM_BOOT];
}
static void write_boot_rom(memaddr, uint8_t) {}
static bus_interface_t bus_boot_rom = { .read = read_boot_rom, .write = write_boot_rom };

void init_bootrom(gb_bus_t *bus) {
    bus->interface_rom_boot = &bus_boot_rom;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions

###############################################################################
############################################################################ */

gb_return_t
GB_load_bootrom(const uint8_t *data, size_t size)
{
    if (size != GB_DMG_BOOT_ROM_SIZE)
        return GB_ERROR_BOOTROM_SIZE;
    memcpy(boot_rom, data, GB_DMG_BOOT_ROM_SIZE);
    return GB_RETURN_OK;
}
