/* -- The more directly hardware-related stuffs -- */

#ifndef GB_SPECIFICATION
#define GB_SPECIFICATION

#define KiB                                     1024
#define BYTES                                   1

// DMG - Original Gameboy
#define GB_DMG_WRAM_SIZE                        ( 8 * KiB)
#define GB_DMG_VRAM_SIZE                        ( 8 * KiB)
#define GB_DMG_HRAM_SIZE                        (127 * BYTES)
#define GB_DMG_OAM_SIZE                         (160 * BYTES)

// CGB - Color Game Boy
#define GB_CGB_WRAM_SIZE                        (32 * KiB)
#define GB_CGB_VRAM_SIZE                        (16 * KiB)

// Boot ROM
#define BOOT_ROM                                "bootroms/dmg_boot.bin"
#define GB_DMG_BOOT_ROM_SIZE                    (256 * BYTES)

// Cartridge
#define CARTRIDGE_ROM                           "dmg-acid2.gb"
#define ROM_SIZE                                (32 * KiB)

// LCD Display and PPU (Picture Processing Unit)
#define LCD_WIDTH                               160
#define LCD_HEIGHT                              144

#define DOTS_PER_SECOND                         4194304             // 2^22 = 4 Mihz
#define DOTS_PER_FRAME                          70224
#define DOTS_PER_SCANLINE                       456
#define DOTS_PER_MACHINE_CYCLE                  4
#define MACHINE_CYCLES_PER_FRAME                17556
#define SCANLINES_PER_FRAME                     154
#define SCANLINE_MAX_OBJS                       10

// Memory map
#define ADDR_START_ROM_BOOT                     0x0000
#define ADDR_END_ROM_BOOT                           0x00FF
#define ADDR_START_ROM_FIXED                    0x0000
#define ADDR_END_ROM_FIXED                          0x3FFF
#define ADDR_START_ROM_BANK                     0x4000
#define ADDR_END_ROM_BANK                           0x7FFF
#define ADDR_START_VRAM                         0x8000
#define ADDR_END_VRAM                               0x9FFF
#define ADDR_START_WRAM_CARTRIDGE               0xA000
#define ADDR_END_WRAM_CARTRIDGE                     0xBFFF
#define ADDR_START_WRAM_1                       0xC000
#define ADDR_END_WRAM_1                             0xCFFF
#define ADDR_START_WRAM_2                       0xD000
#define ADDR_END_WRAM_2                             0xDFFF
#define ADDR_START_ECHO_MEM                     0xE000
#define ADDR_END_ECHO_MEM                           0xFDFF
#define ADDR_START_OAM_MEM                      0xFE00
#define ADDR_END_OAM_MEM                            0xFE9F
#define ADDR_START_UNUSABLE                     0xFEA0
#define ADDR_END_UNUSABLE                           0xFEFF
#define ADDR_START_IO                           0xFF00
#define ADDR_END_IO                                 0xFF7F
#define ADDR_START_HRAM                         0xFF80
#define ADDR_END_HRAM                                0xFFFE

// Addresses of interrupt handlers
#define ADDR_INTERRUPT_VBLANK                   0x0040
#define ADDR_INTERRUPT_STAT                     0x0048
#define ADDR_INTERRUPT_TIMER                    0x0050
#define ADDR_INTERRUPT_SERIAL                   0x0058
#define ADDR_INTERRUPT_JOYPAD                   0x0060

/* -- VRAM regions for tile data and tile maps --
Blocks: 128 tiles x 16 bytes each = 2048 KiB / block
Maps: : 32x32 tile indices        = 1024 KiB / map */

#define ADDR_TILE_DATA_BLOCK_0                  0x8000      
#define ADDR_TILE_DATA_BLOCK_1                  0x8800
#define ADDR_TILE_DATA_BLOCK_2                  0x9000
#define ADDR_TILE_MAP_0                         0x9800
#define ADDR_TILE_MAP_1                         0x9C00

/* --Addressable hardware registers-- */

// Interrupt flags and individual interrupt enables
#define MEMADDR_IF                              0xFF0F
#define MEMADDR_IE                              0xFFFF

// Only writeable to unmap boot ROM from memory once
#define MEMADDR_BOOT_ROM_LOCK                   0xFF50

// PPU registers
#define MEMADDR_LCDC                            0xFF40
#define MEMADDR_STAT                            0xFF41
#define MEMADDR_SCY                             0xFF42
#define MEMADDR_SCX                             0xFF43
#define MEMADDR_LY                              0xFF44
#define MEMADDR_LYC                             0xFF45
#define MEMADDR_BGP                             0xFF47
#define MEMADDR_OBP0                            0xFF48
#define MEMADDR_OBP1                            0xFF49
#define MEMADDR_WY                              0xFF4A
#define MEMADDR_WX                              0xFF4B

// Timer registers
#define MEMADDR_DIV                             0xFF04
#define MEMADDR_TIMA                            0xFF05
#define MEMADDR_TMA                             0xFF06
#define MEMADDR_TAC                             0xFF07


// Because of hardware design, default when memory is inaccessible is 1 bits
#define UNREADABLE                              0xFF

#endif