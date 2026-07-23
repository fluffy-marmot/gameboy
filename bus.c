#include "bus.h"

#include <string.h>

#define KiB                         1024

#define GB_DMG_WRAM                 ( 8 * KiB)
#define GB_CGB_WRAM                 (32 * KiB)
#define GB_DMG VRAM                 ( 8 * KiB)
#define GB_CGB VRAM                 (16 * KiB)

// Boot ROM lock register (aka BANK register) values
#define FF50_BOOT_ROM_ACTIVE        0b00000000
#define FF50_BOOT_ROM_INACTIVE      0b00000001

#define MASK_ECHO_RAM               0b0001111111111111
#define UNREADABLE                  0xFF

#define ADDR_RANGE(low, high) ((low <= address) && (address <= high))


/* ############### Temp stuff for testing ################# */

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
init_test_bus(bus_interface_t *bus)
{
    bus->read = memory_read;
    bus->write = memory_write;
}

/* ################################ */

gb_bus_t bus;

bus_interface_t *
select_interface(memaddr address)
{
    if (ADDR_RANGE(0x0000, GB_DMG_BOOT_ROM_SIZE))
        if (bus.BOOT_ROM_LOCK == FF50_BOOT_ROM_ACTIVE)
                                                        return bus.interface_rom_boot;       // 256 B (DMG)
    if (ADDR_RANGE(0x0000, 0x3FFF))                     return bus.interface_rom_fixed;      // 16 KiB
    if (ADDR_RANGE(0x4000, 0x7FFF))                     return bus.interface_rom_bank;       // 16 KiB
    if (ADDR_RANGE(0x8000, 0x9FFF))                     return bus.interface_vram;           //  8 KiB
    if (ADDR_RANGE(0xA000, 0xBFFF))                     return bus.interface_wram_extern;    //  8 KiB
    if (ADDR_RANGE(0xC000, 0xCFFF))                     return bus.interface_wram_system;    //  4 KiB
    if (ADDR_RANGE(0xD000, 0xDFFF))                     return bus.interface_wram_system;    //  4 KiB
    if (ADDR_RANGE(0xE000, 0xFDFF))                     return bus.interface_echo;
    if (ADDR_RANGE(0xFE00, 0xFE9F))                     return bus.interface_oam;            // 160 B
    if (ADDR_RANGE(0xFEA0, 0xFEFF))                     return bus.interface_unusable;
    if (ADDR_RANGE(0xFF00, 0xFEFF))                     return bus.interface_unusable;
    if (ADDR_RANGE(0xFF80, 0xFFFE))                     return bus.interface_hram;

    /* FF00	FF7F	I/O Registers	
    FF80	FFFE	High RAM (HRAM)	
    FFFF	FFFF	Interrupt Enable register (IE) */
    
}

uint8_t read_bus_FF50(memaddr)          { return UNREADABLE;                                  }
void   write_bus_FF50(memaddr, uint8_t) { bus.BOOT_ROM_LOCK = IMPLEMENTED_BITS_BOOT_ROM_LOCK; }
static bus_interface_t bus_boot_lock =  { read_bus_FF50, write_bus_FF50 };



uint8_t
read_bus_dispatch(memaddr address)
{
    return mem[address];
}

void
write_bus_dispatch(memaddr address, uint8_t value)
{
    mem[address] = value;
}

static bus_interface_t bus_dispatcher = { .read = read_bus_dispatch, .write = write_bus_dispatch };

gb_bus_t *
init_gameboy_bus(void)
{
    bus.bus_dispatcher = &bus_dispatcher;

    bus.interface_register_FF50 = &bus_boot_lock;
}