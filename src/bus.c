#include "_specification.h"
#include "bus.h"
#include "dma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOOT_ROM_ACTIVE                         0b00000000
#define BOOT_ROM_DISABLED                       0b00000001
#define USEPINS_BOOT_ROM_LOCK                   0b00000001

#define MASK_ECHO_RAM                           0b0001111111111111

#define ADDR_RANGE(low, high)                   ((low <= address) && (address <= high))
#define ADDRESS_FF_BUS(address)                 ((address & 0xFF00) == 0xFF00)


static gb_bus_t bus;
static uint8_t *mem_test;
static uint8_t mem_wram[GB_DMG_WRAM_SIZE];
static uint8_t mem_hram[GB_DMG_HRAM_SIZE];

// main dispatch functions forward declaration
uint8_t read_bus_dispatch (memaddr);
static void write_bus_dispatch(memaddr, uint8_t);

// alternate dispatch functions to use while running tests with a simple memory layout
uint8_t test_memory_read (memaddr addr) {
    return mem_test[addr];
}
void test_memory_write(memaddr addr, uint8_t value) {
    mem_test[addr] = value;
}
void test_memory_wipe() {
    if (mem_test != NULL) 
        memset(mem_test, 0, 64 * KiB);
}

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

uint8_t nop_read(memaddr) {}
void    nop_write(memaddr, uint8_t) {}
static  bus_interface_t bus_nop = { .read = nop_read, .write = nop_write };

// Based on address, dispatch the request to the correct interface
static bus_interface_t *
select_interface(memaddr address)
{
    if (ADDR_RANGE(ADDR_START_ROM_BOOT, ADDR_END_ROM_BOOT))
        if (bus.BOOT_ROM_LOCK == BOOT_ROM_ACTIVE)
                                                                        return bus.interface_rom_boot;
    if (ADDR_RANGE(ADDR_START_ROM_FIXED, ADDR_END_ROM_FIXED))           return bus.interface_rom_fixed;
    if (ADDR_RANGE(ADDR_START_ROM_BANK, ADDR_END_ROM_BANK))             return bus.interface_rom_bank;
    if (ADDR_RANGE(ADDR_START_VRAM, ADDR_END_VRAM))                     return bus.interface_vram;
    if (ADDR_RANGE(ADDR_START_WRAM_CARTRIDGE, ADDR_END_WRAM_CARTRIDGE)) return bus.interface_wram_extern;
    if (ADDR_RANGE(ADDR_START_WRAM_1, ADDR_END_WRAM_1))                 return bus.interface_wram_system;
    if (ADDR_RANGE(ADDR_START_WRAM_2, ADDR_END_WRAM_2))                 return bus.interface_wram_system;
    if (ADDR_RANGE(ADDR_START_ECHO_MEM, ADDR_END_ECHO_MEM))             return bus.interface_echo;
    if (ADDR_RANGE(ADDR_START_OAM_MEM, ADDR_END_OAM_MEM))               return bus.interface_oam;
    if (ADDR_RANGE(ADDR_START_UNUSABLE, ADDR_END_UNUSABLE))             return bus.interface_unusable;
    if (ADDR_RANGE(ADDR_START_HRAM, ADDR_END_HRAM))                     return bus.interface_hram;

    switch (address) {
    case MEMADDR_BOOT_ROM_LOCK:                                         return bus.interface_reg_bootlock;

    case MEMADDR_IF:                                    
    case MEMADDR_IE:                                                    return bus.interface_reg_interrupt;

    case MEMADDR_DMA:                                                   return bus.interface_dma;

    case MEMADDR_DIV:
    case MEMADDR_TIMA:
    case MEMADDR_TMA:
    case MEMADDR_TAC:                                                   return bus.interface_reg_timers;

    case MEMADDR_LCDC:
    case MEMADDR_STAT:
    case MEMADDR_SCY:
    case MEMADDR_SCX:
    case MEMADDR_LY:
    case MEMADDR_LYC :
    case MEMADDR_BGP:
    case MEMADDR_OBP0:
    case MEMADDR_OBP1:
    case MEMADDR_WY:
    case MEMADDR_WX:                                                    return bus.interface_reg_ppu;

    default:                                                            return bus.interface_nop;
    }    
}

// The main dispatcher interface using select_interface function to pass on bus requests
uint8_t read_bus_dispatch (memaddr address) {
    if (bus.dma->status == DMA_TRANSFER_ACTIVE && !ADDRESS_FF_BUS(address))
        return bus.dma->data;
    return select_interface(address)->read(address);
}
static void write_bus_dispatch(memaddr address, uint8_t val) {
    if (bus.dma->status == DMA_TRANSFER_ACTIVE && !ADDRESS_FF_BUS(address))
        return;
    select_interface(address)->write(address, val);
}
static bus_interface_t bus_dispatcher = { .read = read_bus_dispatch, .write = write_bus_dispatch };

// WRAM interface
static uint8_t read_wram (memaddr address) {
    return mem_wram[address - ADDR_START_WRAM_1];
}
static void write_wram(memaddr address, uint8_t val) {
    mem_wram[address - ADDR_START_WRAM_1] = val;
}
static bus_interface_t bus_wram = { .read = read_wram, .write = write_wram };

// HRAM interface
static uint8_t read_hram (memaddr address) {
    return mem_hram[address - ADDR_START_HRAM];
}
static void write_hram(memaddr address, uint8_t val) {
    mem_hram[address - ADDR_START_HRAM] = val;
}
static bus_interface_t bus_hram = { .read = read_hram, .write = write_hram };

// Echo RAM interface - mask out highest 3 bits
static uint8_t read_echo(memaddr address) {
    return read_bus_dispatch(address & MASK_ECHO_RAM);
}
static void write_echo (memaddr address, uint8_t val) {
    write_bus_dispatch(address & MASK_ECHO_RAM, val);
}
static bus_interface_t bus_echo =  { .read = read_echo, .write = write_echo };

// Boot ROM lock register - can only be written and turned on to unmap boot ROM
static uint8_t read_reg_bootlock(memaddr) { 
    return UNREADABLE;     
}
static void write_reg_bootlock(memaddr, uint8_t) {
    bus.BOOT_ROM_LOCK = USEPINS_BOOT_ROM_LOCK;
}
static bus_interface_t bus_reg_boot_lock = { .read = read_reg_bootlock, .write = write_reg_bootlock };

// Unusable memory region interface, not much to do
static uint8_t read_unusable(memaddr) {
    return UNREADABLE;
}
static void write_unusable(memaddr, uint8_t) {}
static bus_interface_t bus_unusable = { .read = read_unusable, .write = write_unusable };

gb_bus_t *
init_gameboy_bus(void)
{
    bus.bus_dispatcher = &bus_dispatcher;
    bus.interface_wram_system = &bus_wram;
    bus.interface_echo = &bus_echo;
    bus.interface_unusable = &bus_unusable;
    bus.interface_hram = &bus_hram;

    bus.interface_reg_bootlock = &bus_reg_boot_lock;
    bus.interface_nop = &bus_nop;

    return &bus;
}