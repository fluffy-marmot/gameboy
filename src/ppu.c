#include "ppu.h"

#define MEMADDR_LCDC                0xFF40
#define MEMADDR_STAT                0xFF41
#define MEMADDR_SCY                 0xFF42
#define MEMADDR_SCX                 0xFF43
#define MEMADDR_LY                  0xFF44
#define MEMADDR_LYC                 0xFF45
#define MEMADDR_BGP                 0xFF47
#define MEMADDR_OBP0                0xFF48
#define MEMADDR_OBP1                0xFF49
#define MEMADDR_WY                  0xFF4A
#define MEMADDR_WX                  0xFF4B

#define USEPINS_STAT                0b01111111
#define WRITEABLE_STAT              0b01111000

#define LCDC_BIT_PPU_ENABLE         0b10000000
#define LCDC_BIT_WIN_TILEMAP        0b01000000
#define LCDC_BIT_WIN_ENABLE         0b00100000
#define LCDC_BIT_BG_WIN_TILES       0b00010000
#define LCDC_BIT_BG_TILE_MAP        0b00001000
#define LCDC_BIT_OBJ_HEIGHT         0b00000100
#define LCDC_BIT_OBJ_ENABLE         0b00000010
#define LCDC_BIT_BG_WIN_PRIORITY    0b00000001

#define OBJ_HEIGHT                  ((ppu.LCDC & LCDC_BIT_OBJ_HEIGHT) >> 2)

#define PPU_MODE_MASK               0b00000011
#define PPU_MODE                    (ppu.STAT & PPU_MODE_MASK)
#define PPU_MODE_SET(mode)          (ppu.STAT = (ppu.STAT & (PPU_MODE_MASK ^ 0xFF)) | (mode))
#define PPU_MODE_HBLANK             0b00000000
#define PPU_MODE_VBLANK             0b00000001
#define PPU_MODE_OAM_SCAN           0b00000010
#define PPU_MODE_DRAWING            0b00000011

typedef enum {
    GET_TILE,
    GET_DATA_LOW,
    GET_DATA_HIGH,
    SLEEP
} bg_fetcher_mode;

typedef struct {
    bg_fetcher_mode mode;

    uint8_t x;
    uint8_t y;

    
} bg_fetcher_t;

static gb_ppu_t ppu;

static uint8_t
read_ppu_reg(memaddr address)
{
    switch (address) {
    case MEMADDR_LCDC:              return ppu.LCDC;
    case MEMADDR_STAT:              return ppu.STAT | (USEPINS_STAT ^ 0xFF);
    case MEMADDR_SCY:               return ppu.SCY;
    case MEMADDR_SCX:               return ppu.SCX;
    case MEMADDR_LY:                return ppu.LY;
    case MEMADDR_LYC:               return ppu.LYC;
    case MEMADDR_BGP:               return ppu.BGP;
    case MEMADDR_OBP0:              return ppu.OBP0;
    case MEMADDR_OBP1:              return ppu.OBP1;
    case MEMADDR_WY:                return ppu.WY;
    case MEMADDR_WX:                return ppu.WX;
    default:                        return 0xFF;
    }       
}

static void
write_ppu_reg(memaddr address, uint8_t val)
{
    switch (address) {
    case MEMADDR_LCDC:              ppu.LCDC = val;
    case MEMADDR_STAT:              ppu.STAT = (val & WRITEABLE_STAT) | (USEPINS_STAT ^ 0xFF);
    case MEMADDR_SCY:               ppu.SCY = val;
    case MEMADDR_SCX:               ppu.SCX = val;
    case MEMADDR_LY:                break;  // read-only                                    
    case MEMADDR_LYC:               ppu.LYC = val;
    case MEMADDR_BGP:               ppu.BGP = val;
    case MEMADDR_OBP0:              ppu.OBP0 = val;
    case MEMADDR_OBP1:              ppu.OBP1 = val;
    case MEMADDR_WY:                ppu.WY = val;
    case MEMADDR_WX:                ppu.WX = val;
    }
}
static bus_interface_t bus_registers_ppu = { .read = read_ppu_reg, .write = write_ppu_reg };

static uint8_t
read_vram(memaddr address)
{
    return ppu.vram[address - ADDR_STR_PPUVRAM];
}
static void
write_vram(memaddr address, uint8_t val)
{
    ppu.vram[address - ADDR_STR_PPUVRAM] = val;
}
static bus_interface_t bus_vram = { .read = read_vram, .write = write_vram };

static uint8_t
read_oam(memaddr address)
{
    return ppu.oam[address - ADDR_STR_MEM_OAM];
}
static void
write_oam(memaddr address, uint8_t val)
{
    ppu.oam[address - ADDR_STR_MEM_OAM] = val;
}
static bus_interface_t bus_oam = { .read = read_oam, .write = write_oam };


gb_ppu_t *
init_gameboy_ppu(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    ppu.irq = irq;
    bus->interface_vram = &bus_vram;
    bus->interface_oam = &bus_oam;
    bus->interface_registers_ppu = &bus_registers_ppu;
    return &ppu;
}

static void
oam_scan_step(void)
{
    uint8_t obj_index = ((ppu.frame_dot %  DOTS_PER_SCANLINE) / 2);
    int16_t obj_y = (ppu.oam[obj_index * 4]) - 16;
    if (obj_y <= ppu.LY && ppu.LY < obj_y + 8 * (OBJ_HEIGHT + 1))
        ppu.oam_scan_indices[ppu.oam_scan_count++] = obj_index;
}

void
dot_cycle(void)
{
    switch (PPU_MODE) {
    case PPU_MODE_OAM_SCAN:
        if (ppu.frame_dot % 2 == 0 && ppu.oam_scan_count < SCANLINE_MAX_OBJS)
            oam_scan_step();
        break;
    }

    ppu.frame_dot++;
    if (ppu.frame_dot % DOTS_PER_SCANLINE == 0) {
        ppu.LY++;
        if (ppu.LY > SCANLINES_PER_FRAME) {
            // finished a frame
            ppu.LY = 0;
            ppu.frame_dot = 0;
            ppu.window_condition = false;
        }

        // do i need vblank interrupts? will do later
        PPU_MODE_SET(ppu.LY > LCD_HEIGHT ? PPU_MODE_VBLANK : PPU_MODE_OAM_SCAN);
        ppu.oam_scan_count = 0;
    }
}


// $8000-$97FF Tile Data - 16 bytes per tile, 384 tiles (3 blocks of 128)
// $9800-$9BFF and $9C00-$9FFF - two 32x32 tile maps for background or window

// $FF40	$FF4B 12 bytes of i/o registers LCD Control, Status, Position, Scrolling, and Palettes (8 palettes?)

/*
FF40 LCD Control 
7   6   5   4   3   2   1   0 
LCD & PPU enable    Window tile map Window enable   BG & Window tiles   BG tile map    OBJ size    OBJ enable  BG & Window enable / priority
I think its all writeable?
https://gbdev.io/pandocs/LCDC.html#ff40--lcdc-lcd-control

FF41 STAT: LCD status - uses bits 6-0 (bits 2, 1 and 0 are read only)
6	5	4	3		                                                         2           1	0
LYC int select	Mode 2 int select	Mode 1 int select	Mode 0 int select	LYC == LY	PPU mode
https://gbdev.io/pandocs/STAT.html

FF42 SCY vertical scroll for background (and objs?)

FF43 SCX horizontal scroll for background (and objs?)

FF44 - LY LCD Y coordinate (read only)

FF45 - LYC: LY compare The Game Boy constantly compares the value of the LYC and LY registers. When both values
are identical, the “LYC=LY” flag in the STAT register is set, and (if enabled) a STAT interrupt is requested.

FF47 BGP (background palette) This register assigns gray shades to the color indices of the BG and Window tiles
Color for...    ID 3    ID 2    ID 1    ID 0 (bits 76 54 32 10) 0   White 1   Light gray 2   Dark gray 3   Black
R/W

FF48 and FF49 - obj palette 0, 1 - assigns gray shades to color indexes of obj same as BGP but lower 2 bits
ignored thats transparent on obj R/W

FF4A Window Y position R/W
FF4B Window X + 7 position R/W

*/