#ifndef GB_BUS
#define GB_BUS

#include <stdint.h>

typedef uint16_t memaddr;

typedef struct {
    uint8_t (*read )(memaddr         );
    void    (*write)(memaddr, uint8_t);
} bus_interface_t;

typedef struct {
    bus_interface_t *bus_dispatcher;

    bus_interface_t *interface_rom_boot;
    bus_interface_t *interface_rom_fixed;
    bus_interface_t *interface_rom_bank;
    bus_interface_t *interface_vram;
    bus_interface_t *interface_wram_system;
    bus_interface_t *interface_wram_extern;
    bus_interface_t *interface_echo;
    bus_interface_t *interface_oam;
    bus_interface_t *interface_unusable;
    bus_interface_t *interface_hram;

    bus_interface_t *interface_reg_bootlock;
    bus_interface_t *interface_reg_ppu;
    bus_interface_t *interface_reg_interrupt;
    bus_interface_t *interface_reg_timers;

    bus_interface_t *interface_nop;

    uint8_t BOOT_ROM_LOCK;
} gb_bus_t;

gb_bus_t *init_gameboy_bus(void);

// ABI
void test_memory_mode_enable(void);
void test_memory_mode_disable(void);
void test_memory_wipe(void);
void test_memory_write(memaddr, uint8_t);
uint8_t test_memory_read (memaddr);
// TODO need a better way to access memory during testing while keeping read_dispatch static

// ABI

#endif