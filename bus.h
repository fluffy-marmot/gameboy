#ifndef GB_BUS
#define GB_BUS

#include <stdint.h>

/*
8000 - 9FFF VRAM (switchable 0/1 for CGB)                                       8 KiB
A000 - BFFF external ram (on cartridge?)                                        8 KiB   cartridge
C000 - CFFF WRAM (working ram)                                                  4 KiB
D000 - DFFF WRAM (also working ram), switchable on CGB via banks                4 KiB
E000 - FDFF Echo RAM (https://gbdev.io/pandocs/Memory_Map.html#echo-ram)        --
FE00 - FE9F Object Attribute Memory (OAM)                                       160 bytes

*/

/* OAM - y */

// 0xFF40 - LCDC (LCD control)
/*
    b7 - LCD off/on 0/1
    b6 - window tile map area 9800-9BFF/9C00-9FFF 0/1 (so upper VRAM?)
    b5 - window enable off/on 0/1
    b4 - BG and window tile data area: 0 = 8800-97FF, 1 = 8000-8FFF
    b3 - BG tile map area
    b2 - obj size: 0 = 8x8, 1 = 8x16
    b1 - obj enable , on = 1
    b0 - BG & window enable priority
*/

#define GB_DMG_BOOT_ROM_SIZE                    0xFF

#define MEMADDR_IF                              0xFF0F
#define MEMADDR_IE                              0xFFFF
#define MEMADDR_BOOT_ROM_LOCK                   0xFF50

#define IMPLEMENTED_BITS_BOOT_ROM_LOCK          0b00000001

typedef uint16_t memaddr;

typedef struct {
    uint8_t (*read)(memaddr);
    void (*write)(memaddr, uint8_t);
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

    bus_interface_t *interface_register_FF50;
    bus_interface_t *interface_registers_interrupt;

    uint8_t BOOT_ROM_LOCK;                              // FF50 boot ROM lock register
} gb_bus_t;

gb_bus_t *init_gameboy_bus(void);

// void init_test_bus(bus_interface_t *bus); // TODO temp

#endif