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
#define BOOT_ROM                                "romlib/bootroms/dmg_boot.bin"
#define GB_DMG_BOOT_ROM_SIZE                    (256 * BYTES)

// Cartridge
#define CARTRIDGE_ROM                           "romlib/drmario.gb"
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

#define MEMADDR_JOYP                            0xFF00
#define MEMADDR_DMA                             0xFF46

// Interrupt flags and individual interrupt enables
#define MEMADDR_IF                              0xFF0F
#define MEMADDR_IE                              0xFFFF

// Only writeable ONCE to unmap boot ROM from memory
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

/* -- Various conventions relating to cartridge header and MBC -- */

#define CARTRIDGE_HEADER_MBC_TYPE               0x0147
#define CARTRIDGE_HEADER_ROM_SIZE               0x0148
#define CARTRIDGE_HEADER_RAM_SIZE               0x0149

typedef enum {
    MBC_TYPE_NONE_PLAIN_ROM                   = 0x00,
    MBC_TYPE_MBC1                             = 0x01,   
    MBC_TYPE_MBC1_RAM                         = 0x02,   
    MBC_TYPE_MBC1_BBRAM                       = 0x03,   
    MBC_TYPE_MBC2                             = 0x05,   
    MBC_TYPE_MBC2_BBRAM                       = 0x06,   
    MBC_TYPE_NONE_RAM                         = 0x08,   
    MBC_TYPE_NONE_BBRAM                       = 0x09,   
    MBC_TYPE_MMM01                            = 0x0B,   
    MBC_TYPE_MMM01_RAM                        = 0x0C,   
    MBC_TYPE_MMM01_BBRAM                      = 0x0D,
    MBC_TYPE_MBC3_RTCLOCK                     = 0x0F,   
    MBC_TYPE_MBC3_RTCLOCK_BBRAM               = 0x01,
    MBC_TYPE_MBC3                             = 0x11,   
    MBC_TYPE_MBC3_RAM                         = 0x12,   
    MBC_TYPE_MBC3_BBRAM                       = 0x13, 
    MBC_TYPE_MBC5                             = 0x19,   
    MBC_TYPE_MBC5_RAM                         = 0x1A,   
    MBC_TYPE_MBC5_BBRAM                       = 0x1B, 
    MBC_TYPE_MBC5_RUMBLE                      = 0x1C,   
    MBC_TYPE_MBC5_RUMBLE_RAM                  = 0x1D,   
    MBC_TYPE_MBC5_RUMBLE_BBRAM                = 0x1E,     
    MBC_TYPE_MBC6                             = 0x20,   
    MBC_TYPE_MBC7                             = 0x22,   
    MBC_TYPE_POCKET_CAMERA                    = 0XFC,
    MBC_TYPE_BANDAI_TAMAS                     = 0xFD,   
    MBC_TYPE_HuC3                             = 0xFE,   
    MBC_TYPE_HuC1                             = 0xFF  
} mbc_type_t;

// each ROM bank is 16KiB
typedef enum {
    MBC_ROM_SIZE_32KiB                        = 0x00,               //  no banks
    MBC_ROM_SIZE_64KiB                        = 0x01,               //   4 banks   
    MBC_ROM_SIZE_128KiB                       = 0x02,               //   8 banks
    MBC_ROM_SIZE_256KiB                       = 0x03,               //  16 banks 
    MBC_ROM_SIZE_512KiB                       = 0x04,               //  32 banks
    MBC_ROM_SIZE_1MiB                         = 0x05,               //  64 banks
    MBC_ROM_SIZE_2MiB                         = 0x06,               // 128 banks
    MBC_ROM_SIZE_4MiB                         = 0x07,               // 256 banks
    MBC_ROM_SIZE_8MiB                         = 0x08,               // 512 banks
    MBC_ROM_SIZE_1152KiB                      = 0x52,               //  72 banks
    MBC_ROM_SIZE_1280KiB                      = 0x52,               //  80 banks
    MBC_ROM_SIZE_1536KiB                      = 0x52                //  96 banks
} mbc_rom_size_t;

typedef enum {
    MBC_RAM_SIZE_NONE                         = 0x00,               //      none             
    MBC_RAM_SIZE_2KiB                         = 0x01,               //  no banks 
    MBC_RAM_SIZE_8KiB                         = 0x02,               //  no banks
    MBC_RAM_SIZE_32KiB                        = 0x03,               //   4 banks
    MBC_RAM_SIZE_128KiB                       = 0x04,               //  16 banks
    MBC_RAM_SIZE_64KiB                        = 0x05,               //   8 banks 
} mbc_ram_size_t;

// Because of hardware design, default when memory is inaccessible is 1 bits
#define UNREADABLE                              0xFF

#endif