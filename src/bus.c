#include "bus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Boot ROM lock register (aka BANK register) values
#define FF50_BOOT_ROM_ACTIVE        0b00000000
#define FF50_BOOT_ROM_INACTIVE      0b00000001

#define MASK_ECHO_RAM               0b0001111111111111

#define ADDR_RANGE(low, high) ((low <= address) && (address <= high))



static gb_bus_t bus;
static uint8_t *mem_test;
static uint8_t mem_wram[GB_DMG_WRAM_SIZE];
static uint8_t mem_hram[GB_DMG_HRAM_SIZE];

// TODO for now nonstatic for testing
// main dispatch function forward declaration
uint8_t read_bus_dispatch (memaddr);
static void    write_bus_dispatch(memaddr, uint8_t);

// alternate dispatch functions to use while running tests with a simple memory layout
uint8_t test_memory_read (memaddr addr)                      { /*return mem_test[addr];*/  }
void    test_memory_write(memaddr addr, uint8_t value)       { /*mem_test[addr] = value*;*/ }
void    test_memory_wipe(void)        { if (mem_test != NULL) memset(mem_test, 0, 64 * KiB); }
static  bus_interface_t nop_bus = { .read = test_memory_read, .write = test_memory_write };

void
test_memory_mode_enable(void)
{
    mem_test = calloc(64 * KiB, sizeof(uint8_t));
    bus.bus_dispatcher->read = test_memory_read;
    bus.bus_dispatcher->write = test_memory_write;
}

void
test_memory_mode_disable(void)
{
    free(mem_test);
    mem_test = NULL;
    bus.bus_dispatcher->read = read_bus_dispatch;
    bus.bus_dispatcher->write = write_bus_dispatch;
}

static bus_interface_t *
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
    if (ADDR_RANGE(0xFF40, 0xFF4B))                     return bus.interface_registers_ppu;  // TODO add exception for DMA one
    // (ADDR_RANGE(0xFF7F, 0xFEFF))                     # I/O Registers, handled in switch below
    if (ADDR_RANGE(0xFF80, 0xFFFE))                     return bus.interface_hram;

    switch (address) {
    case MEMADDR_BOOT_ROM_LOCK:                         return bus.interface_register_FF50;  // FF50           
    case MEMADDR_IF:                                    
    case MEMADDR_IE:                                    return bus.interface_registers_interrupt;
    default:
        return &nop_bus;
    }    
}

// TODO non static for testing purposes
uint8_t 
read_bus_dispatch (memaddr address)
{
    return select_interface(address)->read(address);
}
static void
write_bus_dispatch(memaddr address, uint8_t val)
{
    select_interface(address)->write(address, val);
}
static bus_interface_t bus_dispatcher = { .read = read_bus_dispatch, .write = write_bus_dispatch };


static uint8_t
read_wram (memaddr address)
{
    return mem_wram[address - ADDR_STR_WRAMONE];
}
static void
write_wram(memaddr address, uint8_t val)
{
    mem_wram[address - ADDR_STR_WRAMONE] = val;
}
static bus_interface_t bus_wram_system = { .read = read_wram, .write = write_wram };


static uint8_t
read_hram (memaddr address)
{
    return mem_hram[address - ADDR_STR_HIGHRAM];
}
static void
write_hram(memaddr address, uint8_t val)
{
    mem_hram[address - ADDR_STR_HIGHRAM] = val;
}
static bus_interface_t bus_hram = { .read = read_hram, .write = write_hram };


static uint8_t 
read_echo(memaddr address)
{
    return read_bus_dispatch(address & MASK_ECHO_RAM);
}
static void
write_echo (memaddr address, uint8_t val)
{
    write_bus_dispatch(address & MASK_ECHO_RAM, val);
}
static bus_interface_t bus_echo =  { .read = read_echo, .write = write_echo };


static uint8_t read_bus_FF50(memaddr)          { return UNREADABLE;                                  }
static void   write_bus_FF50(memaddr, uint8_t) { bus.BOOT_ROM_LOCK = IMPLEMENTED_BITS_BOOT_ROM_LOCK; }
static bus_interface_t bus_boot_lock =         { .read = read_bus_FF50, .write = write_bus_FF50      };

static uint8_t read_unusable(memaddr)          { return UNREADABLE;                                  }
static void   write_unusable(memaddr, uint8_t) {}
static bus_interface_t bus_unusable =          { .read = read_unusable, .write = write_unusable      };

gb_bus_t *
init_gameboy_bus(void)
{
    bus.bus_dispatcher = &bus_dispatcher;
    bus.interface_wram_system = &bus_wram_system;
    bus.interface_echo = &bus_echo;
    bus.interface_unusable = &bus_unusable;
    bus.interface_hram = &bus_hram;

    bus.interface_register_FF50 = &bus_boot_lock;

    return &bus;
}