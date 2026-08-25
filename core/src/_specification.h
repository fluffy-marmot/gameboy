//////////////////////////////////////////////////////
/* -- This file contains various hardware info/specs -- */
//////////////////////////////////////////////////////

#ifndef GB_SPECIFICATION
#define GB_SPECIFICATION

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t memaddr;

#define BYTES                                   1
#define KiB                                     1024
#define MiB                                     1024 * KiB

// DMG - Original Gameboy
#define GB_DMG_WRAM_SIZE                        ( 8 * KiB)
#define GB_DMG_VRAM_SIZE                        ( 8 * KiB)
#define GB_DMG_BOOT_ROM_SIZE                    (256 * BYTES)
#define GB_DMG_OAM_SIZE                         (160 * BYTES)
#define GB_DMG_HRAM_SIZE                        (127 * BYTES)
#define GB_DMG_WAVERAM_SIZE                     ( 16 * BYTES)

// CGB - Color Game Boy
#define GB_CGB_WRAM_SIZE                        (32 * KiB)
#define GB_CGB_VRAM_SIZE                        (16 * KiB)

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
#define ADDR_START_APU                          0xFF10
#define ADDR_END_APU                                0xFF3F
#define ADDR_START_WAVERAM                      0xFF30
#define ADDR_END_WAVERAM                            0xFF3F
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

//////////////////////////////////////////////////////
/* --Addressable hardware registers-- */
//////////////////////////////////////////////////////

// Joypad
#define MEMADDR_JOYP                            0xFF00

// Serial registers
#define MEMADDR_SB                              0xFF01
#define MEMADDR_SC                              0xFF02

// Timer registers
#define MEMADDR_DIV                             0xFF04
#define MEMADDR_TIMA                            0xFF05
#define MEMADDR_TMA                             0xFF06
#define MEMADDR_TAC                             0xFF07

// Interrupt flags and individual interrupt enables
#define MEMADDR_IF                              0xFF0F
#define MEMADDR_IE                              0xFFFF

// APU registers
#define MEMADDR_NR52                            0xFF26
#define MEMADDR_NR51                            0xFF25
#define MEMADDR_NR50                            0xFF24

#define MEMADDR_NR10                            0xFF10
#define MEMADDR_NR11                            0xFF11
#define MEMADDR_NR12                            0xFF12
#define MEMADDR_NR13                            0xFF13
#define MEMADDR_NR14                            0xFF14

#define MEMADDR_NR21                            0xFF16
#define MEMADDR_NR22                            0xFF17
#define MEMADDR_NR23                            0xFF18
#define MEMADDR_NR24                            0xFF19

#define MEMADDR_NR30                            0xFF1A
#define MEMADDR_NR31                            0xFF1B
#define MEMADDR_NR32                            0xFF1C
#define MEMADDR_NR33                            0xFF1D
#define MEMADDR_NR34                            0xFF1E

#define MEMADDR_NR41                            0xFF20
#define MEMADDR_NR42                            0xFF21
#define MEMADDR_NR43                            0xFF22
#define MEMADDR_NR44                            0xFF23

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

// Register for triggering DMA transfer
#define MEMADDR_DMA                             0xFF46

// Only writeable ONCE to unmap boot ROM from memory
#define MEMADDR_BOOT_ROM_LOCK                   0xFF50

// Because of hardware design, default when memory is inaccessible is 1 bits
#define UNREADABLE                              0xFF

//////////////////////////////////////////////////////
/* -- Various conventions relating to cartridge header and MBC -- */
//////////////////////////////////////////////////////

#define CARTRIDGE_HEADER_ENTRY_POINT            0x0100              //  4 bytes
#define CARTRIDGE_HEADER_NINTENDO_LOGO          0x0104              // 48 bytes
#define CARTRIDGE_HEADER_TITLE                  0x0134              // 16 bytes
#define CARTRIDGE_HEADER_MANUFACTURER           0x013F              //  4 bytes
#define CARTRIDGE_HEADER_CGB_FLAG               0x0143              //  1 byte
#define CARTRIDGE_HEADER_NEW_LICENSEE_CODE      0x0144              //  2 bytes
#define CARTRIDGE_HEADER_SGB_FLAG               0x0146              //  1 byte
#define CARTRIDGE_HEADER_MBC_TYPE               0x0147              //  1 byte
#define CARTRIDGE_HEADER_ROM_SIZE               0x0148              //  1 byte
#define CARTRIDGE_HEADER_RAM_SIZE               0x0149              //  1 byte
#define CARTRIDGE_HEADER_DESTINATION_CODE       0x014A              //  1 byte
#define CARTRIDGE_HEADER_OLD_LICENSEE_CODE      0x014B              //  1 byte
#define CARTRIDGE_HEADER_ROM_VERSION            0x014C              //  1 byte
#define CARTRIDGE_HEADER_CHECKSUM               0x014D              //  1 byte
#define CARTRIDGE_HEADER_GLOBAL_CHECKSUM        0x014E              //  2 bytes

#define ADDR_START_HEADER_CHECKSUM_RANGE        0x0134
#define ADDR_END_HEADER_CHECKSUM_RANGE          0x014C

typedef enum {
    CGB_FLAG_BACKWARDS_COMPATIBLE             = 0x80,
    CGB_FLAG_CGB_ONLY                         = 0xC0
} header_cgb_flag_t;

typedef enum {
    SGB_FLAG_SGB_FUNCTIONS_SUPPORT            = 0x03
} header_sgb_flag_t;

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
    MBC_TYPE_MBC3_RTCLOCK_BBRAM               = 0x10,
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
    MBC_TYPE_BANDAI_TAMA5                     = 0xFD,
    MBC_TYPE_HuC3                             = 0xFE,
    MBC_TYPE_HuC1                             = 0xFF
} header_mbc_type_t;

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
    MBC_ROM_SIZE_1280KiB                      = 0x53,               //  80 banks
    MBC_ROM_SIZE_1536KiB                      = 0x54                //  96 banks
} header_rom_size_t;

// each RAM bank is 8KiB
typedef enum {
    MBC_RAM_SIZE_NONE                         = 0x00,               //      none
    MBC_RAM_SIZE_2KiB                         = 0x01,               //  no banks
    MBC_RAM_SIZE_8KiB                         = 0x02,               //  no banks
    MBC_RAM_SIZE_32KiB                        = 0x03,               //   4 banks
    MBC_RAM_SIZE_128KiB                       = 0x04,               //  16 banks
    MBC_RAM_SIZE_64KiB                        = 0x05,               //   8 banks
} header_ram_size_t;

typedef enum {
    HEADER_DESTINATION_CODE_JAPAN             = 0x00,
    HEADER_DESTINATION_CODE_OVERSEAS_ONLY     = 0X01
} header_des_code_t;

// licensee enum generated via Claude based on Pandocs list
// https://gbdev.io/pandocs/The_Cartridge_Header.html#014b--old-licensee-code
typedef enum {
    OLD_LICENSEE_NONE                         = 0x00,
    OLD_LICENSEE_NINTENDO_01                  = 0x01,
    OLD_LICENSEE_CAPCOM_08                    = 0x08,
    OLD_LICENSEE_HOT_B                        = 0x09,
    OLD_LICENSEE_JALECO_0A                    = 0x0A,
    OLD_LICENSEE_COCONUTS_JAPAN               = 0x0B,
    OLD_LICENSEE_ELITE_SYSTEMS_0C             = 0x0C,
    OLD_LICENSEE_EA_ELECTRONIC_ARTS_13        = 0x13,
    OLD_LICENSEE_HUDSON_SOFT                  = 0x18,
    OLD_LICENSEE_ITC_ENTERTAINMENT            = 0x19,
    OLD_LICENSEE_YANOMAN                      = 0x1A,
    OLD_LICENSEE_JAPAN_CLARY                  = 0x1D,
    OLD_LICENSEE_VIRGIN_GAMES_LTD_1F          = 0x1F,
    OLD_LICENSEE_PCM_COMPLETE                 = 0x24,
    OLD_LICENSEE_SAN_X                        = 0x25,
    OLD_LICENSEE_KEMCO_28                     = 0x28,
    OLD_LICENSEE_SETA_CORPORATION             = 0x29,
    OLD_LICENSEE_INFOGRAMES_30                = 0x30,
    OLD_LICENSEE_NINTENDO_31                  = 0x31,
    OLD_LICENSEE_BANDAI_32                    = 0x32,
    OLD_LICENSEE_NEW_LICENSEE_CODE            = 0x33,
    OLD_LICENSEE_KONAMI_34                    = 0x34,
    OLD_LICENSEE_HECTORSOFT                   = 0x35,
    OLD_LICENSEE_CAPCOM_38                    = 0x38,
    OLD_LICENSEE_BANPRESTO_39                 = 0x39,
    OLD_LICENSEE_ENTERTAINMENT_INTERACTIVE    = 0x3C,
    OLD_LICENSEE_GREMLIN                      = 0x3E,
    OLD_LICENSEE_UBI_SOFT                     = 0x41,
    OLD_LICENSEE_ATLUS_42                     = 0x42,
    OLD_LICENSEE_MALIBU_INTERACTIVE_44        = 0x44,
    OLD_LICENSEE_ANGEL_46                     = 0x46,
    OLD_LICENSEE_SPECTRUM_HOLOBYTE            = 0x47,
    OLD_LICENSEE_IREM                         = 0x49,
    OLD_LICENSEE_VIRGIN_GAMES_LTD_4A          = 0x4A,
    OLD_LICENSEE_MALIBU_INTERACTIVE_4D        = 0x4D,
    OLD_LICENSEE_U_S_GOLD                     = 0x4F,
    OLD_LICENSEE_ABSOLUTE                     = 0x50,
    OLD_LICENSEE_ACCLAIM_ENTERTAINMENT_51     = 0x51,
    OLD_LICENSEE_ACTIVISION                   = 0x52,
    OLD_LICENSEE_SAMMY_USA_CORPORATION        = 0x53,
    OLD_LICENSEE_GAMETEK                      = 0x54,
    OLD_LICENSEE_PARK_PLACE                   = 0x55,
    OLD_LICENSEE_LJN_56                       = 0x56,
    OLD_LICENSEE_MATCHBOX                     = 0x57,
    OLD_LICENSEE_MILTON_BRADLEY_COMPANY       = 0x59,
    OLD_LICENSEE_MINDSCAPE                    = 0x5A,
    OLD_LICENSEE_ROMSTAR                      = 0x5B,
    OLD_LICENSEE_NAXAT_SOFT_5C                = 0x5C,
    OLD_LICENSEE_TRADEWEST                    = 0x5D,
    OLD_LICENSEE_TITUS_INTERACTIVE            = 0x60,
    OLD_LICENSEE_VIRGIN_GAMES_LTD_61          = 0x61,
    OLD_LICENSEE_OCEAN_SOFTWARE               = 0x67,
    OLD_LICENSEE_EA_ELECTRONIC_ARTS_69        = 0x69,
    OLD_LICENSEE_ELITE_SYSTEMS_6E             = 0x6E,
    OLD_LICENSEE_ELECTRO_BRAIN                = 0x6F,
    OLD_LICENSEE_INFOGRAMES_70                = 0x70,
    OLD_LICENSEE_INTERPLAY_ENTERTAINMENT      = 0x71,
    OLD_LICENSEE_BRODERBUND_72                = 0x72,
    OLD_LICENSEE_SCULPTURED_SOFTWARE          = 0x73,
    OLD_LICENSEE_THE_SALES_CURVE_LIMITED      = 0x75,
    OLD_LICENSEE_THQ                          = 0x78,
    OLD_LICENSEE_ACCOLADE                     = 0x79,
    OLD_LICENSEE_TRIFFIX_ENTERTAINMENT        = 0x7A,
    OLD_LICENSEE_MICROPROSE                   = 0x7C,
    OLD_LICENSEE_KEMCO_7F                     = 0x7F,
    OLD_LICENSEE_MISAWA_ENTERTAINMENT         = 0x80,
    OLD_LICENSEE_LOZC_G                       = 0x83,
    OLD_LICENSEE_TOKUMA_SHOTEN_86             = 0x86,
    OLD_LICENSEE_BULLET_PROOF_SOFTWARE        = 0x8B,
    OLD_LICENSEE_VIC_TOKAI_CORP               = 0x8C,
    OLD_LICENSEE_APE_INC                      = 0x8E,
    OLD_LICENSEE_IMAX                         = 0x8F,
    OLD_LICENSEE_CHUNSOFT_CO                  = 0x91,
    OLD_LICENSEE_VIDEO_SYSTEM                 = 0x92,
    OLD_LICENSEE_TSUBARAYA_PRODUCTIONS        = 0x93,
    OLD_LICENSEE_VARIE_95                     = 0x95,
    OLD_LICENSEE_YONEZAWA_SPAL                = 0x96,
    OLD_LICENSEE_KEMCO_97                     = 0x97,
    OLD_LICENSEE_ARC                          = 0x99,
    OLD_LICENSEE_NIHON_BUSSAN                 = 0x9A,
    OLD_LICENSEE_TECMO                        = 0x9B,
    OLD_LICENSEE_IMAGINEER                    = 0x9C,
    OLD_LICENSEE_BANPRESTO_9D                 = 0x9D,
    OLD_LICENSEE_NOVA                         = 0x9F,
    OLD_LICENSEE_HORI_ELECTRIC                = 0xA1,
    OLD_LICENSEE_BANDAI_A2                    = 0xA2,
    OLD_LICENSEE_KONAMI_A4                    = 0xA4,
    OLD_LICENSEE_KAWADA                       = 0xA6,
    OLD_LICENSEE_TAKARA                       = 0xA7,
    OLD_LICENSEE_TECHNOS_JAPAN                = 0xA9,
    OLD_LICENSEE_BRODERBUND_AA                = 0xAA,
    OLD_LICENSEE_TOEI_ANIMATION               = 0xAC,
    OLD_LICENSEE_TOHO                         = 0xAD,
    OLD_LICENSEE_NAMCO                        = 0xAF,
    OLD_LICENSEE_ACCLAIM_ENTERTAINMENT_B0     = 0xB0,
    OLD_LICENSEE_ASCII_CORPORATION_OR_NEXSOFT = 0xB1,
    OLD_LICENSEE_BANDAI_B2                    = 0xB2,
    OLD_LICENSEE_SQUARE_ENIX                  = 0xB4,
    OLD_LICENSEE_HAL_LABORATORY               = 0xB6,
    OLD_LICENSEE_SNK                          = 0xB7,
    OLD_LICENSEE_PONY_CANYON_B9               = 0xB9,
    OLD_LICENSEE_CULTURE_BRAIN                = 0xBA,
    OLD_LICENSEE_SUNSOFT                      = 0xBB,
    OLD_LICENSEE_SONY_IMAGESOFT               = 0xBD,
    OLD_LICENSEE_SAMMY_CORPORATION            = 0xBF,
    OLD_LICENSEE_TAITO_C0                     = 0xC0,
    OLD_LICENSEE_KEMCO_C2                     = 0xC2,
    OLD_LICENSEE_SQUARE                       = 0xC3,
    OLD_LICENSEE_TOKUMA_SHOTEN_C4             = 0xC4,
    OLD_LICENSEE_DATA_EAST                    = 0xC5,
    OLD_LICENSEE_TONKIN_HOUSE                 = 0xC6,
    OLD_LICENSEE_KOEI                         = 0xC8,
    OLD_LICENSEE_UFL                          = 0xC9,
    OLD_LICENSEE_ULTRA_GAMES                  = 0xCA,
    OLD_LICENSEE_VAP_INC                      = 0xCB,
    OLD_LICENSEE_USE_CORPORATION              = 0xCC,
    OLD_LICENSEE_MELDAC                       = 0xCD,
    OLD_LICENSEE_PONY_CANYON_CE               = 0xCE,
    OLD_LICENSEE_ANGEL_CF                     = 0xCF,
    OLD_LICENSEE_TAITO_D0                     = 0xD0,
    OLD_LICENSEE_SOFEL                        = 0xD1,
    OLD_LICENSEE_QUEST                        = 0xD2,
    OLD_LICENSEE_SIGMA_ENTERPRISES            = 0xD3,
    OLD_LICENSEE_ASK_KODANSHA_CO              = 0xD4,
    OLD_LICENSEE_NAXAT_SOFT_D6                = 0xD6,
    OLD_LICENSEE_COPYA_SYSTEM                 = 0xD7,
    OLD_LICENSEE_BANPRESTO_D9                 = 0xD9,
    OLD_LICENSEE_TOMY                         = 0xDA,
    OLD_LICENSEE_LJN_DB                       = 0xDB,
    OLD_LICENSEE_NIPPON_COMPUTER_SYSTEMS      = 0xDD,
    OLD_LICENSEE_HUMAN_ENT                    = 0xDE,
    OLD_LICENSEE_ALTRON                       = 0xDF,
    OLD_LICENSEE_JALECO_E0                    = 0xE0,
    OLD_LICENSEE_TOWA_CHIKI                   = 0xE1,
    OLD_LICENSEE_YUTAKA                       = 0xE2,
    OLD_LICENSEE_VARIE_E3                     = 0xE3,
    OLD_LICENSEE_EPOCH                        = 0xE5,
    OLD_LICENSEE_ATHENA                       = 0xE7,
    OLD_LICENSEE_ASMIK_ACE_ENTERTAINMENT      = 0xE8,
    OLD_LICENSEE_NATSUME                      = 0xE9,
    OLD_LICENSEE_KING_RECORDS                 = 0xEA,
    OLD_LICENSEE_ATLUS_EB                     = 0xEB,
    OLD_LICENSEE_EPIC_SONY_RECORDS            = 0xEC,
    OLD_LICENSEE_IGS                          = 0xEE,
    OLD_LICENSEE_A_WAVE                       = 0xF0,
    OLD_LICENSEE_EXTREME_ENTERTAINMENT        = 0xF3,
    OLD_LICENSEE_LJN_FF                       = 0xFF
} header_old_licensee_code_t;

#endif
