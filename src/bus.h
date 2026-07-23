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

#define KiB                                     1024

#define GB_DMG_WRAM_SIZE                        ( 8 * KiB)
#define GB_CGB_WRAM_SIZE                        (32 * KiB)
#define GB_DMG_VRAM_SIZE                        ( 8 * KiB)
#define GB_CGB_VRAM_SIZE                        (16 * KiB)
#define GB_DMG_HRAM_SIZE                        127
#define GB_DMG__OAM_SIZE                        160

#define ADDR_STR_ROMFIXD                        0x0000
#define ADDR_END_ROMFIXD                        0x3FFF
#define ADDR_STR_ROMBANK                        0x4000
#define ADDR_END_ROMBANK                        0x7FFF
#define ADDR_STR_PPUVRAM                        0x8000
#define ADDR_END_PPUVRAM                        0x9FFF
#define ADDR_STR_WRAMEXT                        0xA000
#define ADDR_END_WRAMEXT                        0xBFFF
#define ADDR_STR_WRAMONE                        0xC000
#define ADDR_END_WRAMONE                        0xCFFF
#define ADDR_STR_WRAMSEC                        0xD000
#define ADDR_END_WRAMSEC                        0xDFFF
#define ADDR_STR_MEMECHO                        0xE000
#define ADDR_END_MEMECHO                        0xFDFF
#define ADDR_STR_MEM_OAM                        0xFE00
#define ADDR_END_MEM_OAM                        0xFE9F
#define ADDR_STR_UNUSBLE                        0xFEA0
#define ADDR_END_UNUSBLE                        0xFEFF
#define ADDR_STR_IO_REGS                        0xFF00
#define ADDR_END_IO_REGS                        0xFF7F
#define ADDR_STR_HIGHRAM                        0xFF80
#define ADDR_END_HIGHRAM                        0xFFFE

#define GB_DMG_BOOT_ROM_SIZE                    0xFF

#define MEMADDR_IF                              0xFF0F
#define MEMADDR_IE                              0xFFFF
#define MEMADDR_BOOT_ROM_LOCK                   0xFF50

#define UNREADABLE                              0xFF

#define IMPLEMENTED_BITS_BOOT_ROM_LOCK          0b00000001

typedef uint16_t memaddr;

typedef struct {
    uint8_t (*read)(memaddr);
    void (*write)(memaddr, uint8_t);
} bus_interface_t;

typedef struct {
    bus_interface_t *bus_dispatcher;            //

    bus_interface_t *interface_rom_boot;        //
    bus_interface_t *interface_rom_fixed;       // cartridge todo
    bus_interface_t *interface_rom_bank;        // cartridge todo
    bus_interface_t *interface_vram;            // ppu todo
    bus_interface_t *interface_wram_system;     //
    bus_interface_t *interface_wram_extern;     // cartridge todo
    bus_interface_t *interface_echo;            //
    bus_interface_t *interface_oam;             // ppu todo
    bus_interface_t *interface_unusable;        //
    bus_interface_t *interface_hram;            //

    bus_interface_t *interface_register_FF50;       //
    bus_interface_t *interface_registers_ppu;       //
    bus_interface_t *interface_registers_interrupt; //

    uint8_t BOOT_ROM_LOCK;                              // FF50 boot ROM lock register
} gb_bus_t;

gb_bus_t *init_gameboy_bus(void);

// use these functions for a flat 64 KiB memory layout, for e.g. simple single instruction sm83 tests
void test_memory_mode_enable(void);
void test_memory_mode_disable(void);
void test_memory_wipe(void);
void test_memory_write(memaddr, uint8_t);
uint8_t test_memory_read (memaddr);

#endif