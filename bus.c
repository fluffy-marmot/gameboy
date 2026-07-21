#include "bus.h"

#include <string.h>

#define KiB                         1024

#define GB_DMG_WRAM             ( 8 * KiB)
#define GB_CGB_WRAM             (32 * KiB)
#define GB_DMG VRAM             ( 8 * KiB)
#define GB_CGB VRAM             (16 * KiB)

uint8_t mem[64 * KiB];

uint8_t
memory_read(memaddr address)
{
    return mem[address];
}

void
memory_write(memaddr address, uint8_t value)
{
    mem[address] = value;
}

void
memory_wipe(void)
{
    memset(mem, 0, 64 * KiB);
}

void
init_test_bus(gb_bus_t *bus)
{
    bus->read = memory_read;
    bus->write = memory_write;
}