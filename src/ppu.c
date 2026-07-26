#include "ppu.h"

#include <stdio.h>
#include <string.h>

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

#define STAT_BIT_LYC_INT_SELECT     0b01000000
#define STAT_BIT_MD2_INT_SELECT     0b00100000
#define STAT_BIT_MD1_INT_SELECT     0b00010000
#define STAT_BIT_MD0_INT_SELECT     0b00001000
#define STAT_BIT_EQL                0b00000100

#define OAM_FLAG_BIT_PRIORITY       0b10000000
#define OAM_FLAG_BIT_YFLIP          0b01000000
#define OAM_FLAG_BIT_XFLIP          0b00100000
#define OAM_FLAG_BIT_PALETTE        0b00010000

// 8px or 16px sprite heights
#define OBJ_HEIGHT                  ((((ppu.LCDC & LCDC_BIT_OBJ_HEIGHT) >> 2) + 1) * 8)
#define BG_TILE_MAP                 (((ppu.LCDC & LCDC_BIT_BG_TILE_MAP) >> 3) ? 0x9C00 : 0x9800)
#define BG_TILE_DATA_METHOD         ((ppu.LCDC & LCDC_BIT_BG_WIN_TILES) >> 4)
#define BG_TILE_DATA_METHOD_8000    0b00000001
#define BG_TILE_DATA_METHOD_8800    0b00000000

#define STAT_COMPARE_LYC_LY         (ppu.STAT = (ppu.STAT & (STAT_BIT_EQL ^ 0xFF)) | ((ppu.LY == ppu.LYC) << 2))

#define PPU_MODE_MASK               0b00000011
#define PPU_MODE                    (ppu.STAT & PPU_MODE_MASK)
#define PPU_MODE_SET(mode)          (ppu.STAT = (ppu.STAT & (PPU_MODE_MASK ^ 0xFF)) | (mode))
#define PPU_MODE_HBLANK             0b00000000
#define PPU_MODE_VBLANK             0b00000001
#define PPU_MODE_OAM_SCAN           0b00000010
#define PPU_MODE_DRAWING            0b00000011

#define OAM_Y(i)                    (ppu.oam[(i) * 4])
#define OAM_X(i)                    (ppu.oam[(i) * 4 + 1])
#define OAM_TILE_INDEX(i)           (ppu.oam[(i) * 4 + 2])
#define OAM_FLAGS(i)                (ppu.oam[(i) * 4 + 3])

#define OAM_PRIORITY(i)             ((OAM_FLAGS(i) & OAM_FLAG_BIT_PRIORITY) >> 7)
#define OAM_YFLIP(i)                ((OAM_FLAGS(i) & OAM_FLAG_BIT_YFLIP) >> 6)
#define OAM_XFLIP(i)                ((OAM_FLAGS(i) & OAM_FLAG_BIT_XFLIP) >> 5)
#define OAM_PALETTE(i)              ((OAM_FLAGS(i) & OAM_FLAG_BIT_PALETTE) >> 4)

#define VRAM(address)               (ppu.vram[(address) - ADDR_STR_PPUVRAM])
#define  OAM(address)               (ppu.oam [(address) - ADDR_STR_MEM_OAM])

typedef enum {
    GET_TILE,
    GET_DATA_LOW,
    GET_DATA_HIGH,
    PUSH,
    CHECK_X,
    IDLE
} fetcher_mode;

typedef struct {
    uint8_t color;
    uint8_t palette;
    uint8_t bg_priority;
} fifo_pixel_t;

static struct {
    fetcher_mode mode;
    uint8_t delay;
    uint8_t data_low;
    uint8_t data_high;

    uint16_t tile;
    uint8_t tile_number;
    uint8_t win_line_counter;
} bg_fetcher;

static struct {
    fetcher_mode mode;
    uint8_t delay;
    uint8_t data_low;
    uint8_t data_high;

    uint8_t searched;
    bool index_used[10];
    uint16_t tile_number;
} obj_fetcher;

static struct {
    uint8_t warmup;
    uint8_t discard;

    fifo_pixel_t obj_pixels[8];
    // uint8_t obj_pixels_len;
    uint8_t bg_pixels[8];
    uint8_t bg_pixels_len;
} pixel_mixer;

static const uint32_t LCD_COLORS[4] = {
    0x00F4FFBF,
    0x009AD941,
    0x00608057,
    0x00002B40
};



static gb_ppu_t ppu;

static void
check_lyc_interrupt(void)
{
    STAT_COMPARE_LYC_LY;
    if (ppu.LY == ppu.LYC) {
        ppu.irq->IF |= 2;
    }
}

static uint8_t
read_ppu_reg(memaddr address)
{
    switch (address) {
    case MEMADDR_LCDC:              return ppu.LCDC;
    case MEMADDR_STAT:              return ppu.STAT | (USEPINS_STAT ^ 0xFF);
    case MEMADDR_SCY:               return ppu.SCY;
    case MEMADDR_SCX:               return ppu.SCX;
    case MEMADDR_LY:                return 0x90; //return ppu.LY;
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
    case MEMADDR_LCDC:              ppu.LCDC = val;                                                 break;
    case MEMADDR_STAT:              ppu.STAT = (val & WRITEABLE_STAT) | (USEPINS_STAT ^ 0xFF);      break;
    case MEMADDR_SCY:               ppu.SCY = val;                                                  break;
    case MEMADDR_SCX:               ppu.SCX = val;                                                  break;
    case MEMADDR_LY:                                                                                break;                                 
    case MEMADDR_LYC:               ppu.LYC = val; check_lyc_interrupt();                           break;
    case MEMADDR_BGP:               ppu.BGP = val;                                                  break;
    case MEMADDR_OBP0:              ppu.OBP0 = val;                                                 break;
    case MEMADDR_OBP1:              ppu.OBP1 = val;                                                 break;
    case MEMADDR_WY:                ppu.WY = val;                                                   break;
    case MEMADDR_WX:                ppu.WX = val;                                                   break;
    }
}
static bus_interface_t bus_registers_ppu = { .read = read_ppu_reg, .write = write_ppu_reg };

static uint8_t
read_vram(memaddr address)
{
    return VRAM(address);
}
static void
write_vram(memaddr address, uint8_t val)
{
    VRAM(address) = val;
}
static bus_interface_t bus_vram = { .read = read_vram, .write = write_vram };

static uint8_t
read_oam(memaddr address)
{
    return OAM(address);
}
static void
write_oam(memaddr address, uint8_t val)
{
    OAM(address) = val;
}
static bus_interface_t bus_oam = { .read = read_oam, .write = write_oam };
// TODO when is vram / oam access blocked and to what?

static void
oam_scan_step(void)
{
    uint8_t obj_index = ((ppu.frame_dot % DOTS_PER_SCANLINE) / 2);
    // dont add objs with X 0
    if (OAM_X(obj_index) == 0) 
        return;      
    if (OAM_Y(obj_index) <= ppu.LY + 16 && ppu.LY + 16 < OAM_Y(obj_index) + OBJ_HEIGHT)
        ppu.oam_scan_indices[ppu.oam_scan_count++] = obj_index;
}

static void
begin_draw_mode(void)
{
    ppu.lx = 0;

    obj_fetcher.mode = CHECK_X;
    obj_fetcher.searched = 0;
    for (int i = 0; i < SCANLINE_MAX_OBJS; i++)
        obj_fetcher.index_used[i] = !(i < ppu.oam_scan_count);

    bg_fetcher.mode = GET_TILE;
    bg_fetcher.delay = 5;
    // if background, what if window?
    bg_fetcher.tile = (((ppu.SCX / 8) & 0x1F) + 32 * (((ppu.LY + ppu.SCY) & 0xFF) / 8)) & 0x03FF;

    pixel_mixer.bg_pixels_len = 0;
    memset(pixel_mixer.obj_pixels, 0, sizeof(pixel_mixer.obj_pixels));
    pixel_mixer.warmup = 12;
    pixel_mixer.discard = ppu.SCX % 8;

    PPU_MODE_SET(PPU_MODE_DRAWING);
}

static void
step_obj_fetcher(void)
{
    if (obj_fetcher.delay && obj_fetcher.delay--) return;
    switch (obj_fetcher.mode) {
    case GET_TILE:
        obj_fetcher.tile_number = OAM_TILE_INDEX(ppu.oam_scan_indices[obj_fetcher.searched]);
        obj_fetcher.mode = GET_DATA_LOW;
        obj_fetcher.delay = 1;
        break;
    case GET_DATA_LOW:
        uint8_t y_offset = 2 * ((ppu.LY + ppu.SCY - OAM_Y(obj_fetcher.searched) + 16) % 8); // 8 high for now
        obj_fetcher.data_low = VRAM(0x8000 + 16 * obj_fetcher.tile_number + y_offset);
        obj_fetcher.mode = GET_DATA_HIGH;
        obj_fetcher.delay = 1;
        break;
    case GET_DATA_HIGH:
        uint8_t y_offset = 2 * ((ppu.LY + ppu.SCY - OAM_Y(obj_fetcher.searched) + 16) % 8); // 8 high for now
        obj_fetcher.data_low = VRAM(0x8000 + 16 * obj_fetcher.tile_number + y_offset + 1);

        // xflip here?
        for (int i = 7; i >= 0; i--) {
            if (pixel_mixer.obj_pixels[7 - i].color) continue;
            pixel_mixer.obj_pixels[7 - i].color = (((obj_fetcher.data_high & (1 << i)) >> i) << 1) |
                                                   ((obj_fetcher.data_low  & (1 << i)) >> i);
            pixel_mixer.obj_pixels[7 - i].bg_priority = OAM_PRIORITY(ppu.oam_scan_indices[obj_fetcher.searched]);
            pixel_mixer.obj_pixels[7 - i].palette = OAM_PALETTE(ppu.oam_scan_indices[obj_fetcher.searched]);
        }
        obj_fetcher.mode = CHECK_X;
        break;
    case CHECK_X:
        for (int i = obj_fetcher.searched; i < SCANLINE_MAX_OBJS; i++, obj_fetcher.searched++) {
            if (obj_fetcher.index_used[i]) continue;
            if (OAM_X(ppu.oam_scan_indices[i]) <= ppu.lx + 8) {
                obj_fetcher.mode = GET_TILE;
                // obj_fetcher.delay = 1; // no delay i think already built in by 1 dot on check mode?
                obj_fetcher.index_used[i] = true;
                break;
            }
        }
        if (obj_fetcher.searched == SCANLINE_MAX_OBJS) {
            obj_fetcher.mode = IDLE;
            bg_fetcher.mode = GET_TILE;
            bg_fetcher.delay = 1;
        }
        break;
    }
}

static void
step_bg_fetcher(void)
{
    if (bg_fetcher.delay && bg_fetcher.delay--) return;
    switch (bg_fetcher.mode) {
    case GET_TILE:
        bg_fetcher.tile_number = VRAM(BG_TILE_MAP + bg_fetcher.tile++);
        bg_fetcher.mode = GET_DATA_LOW;
        bg_fetcher.delay = 1;
        break;
    case GET_DATA_LOW:
        uint8_t y_offset = 2 * ((ppu.LY + ppu.SCY) % 8);
        if (BG_TILE_DATA_METHOD == BG_TILE_DATA_METHOD_8000)
            bg_fetcher.data_low = VRAM(0x8000 + 16 * bg_fetcher.tile_number + y_offset);
        else
            bg_fetcher.data_low = VRAM(0x9000 + 16 * (int8_t) bg_fetcher.tile_number + y_offset);
        bg_fetcher.mode = GET_DATA_HIGH;
        bg_fetcher.delay = 1;
        break;
    case GET_DATA_HIGH:
        // TODO fix the ugliness later
        uint8_t offset = 2 * ((ppu.LY + ppu.SCY) % 8);
        if (BG_TILE_DATA_METHOD == BG_TILE_DATA_METHOD_8000)
            bg_fetcher.data_high = VRAM(0x8000 + 16 * bg_fetcher.tile_number + offset + 1);
        else
            bg_fetcher.data_high = VRAM(0x9000 + 16 * (int8_t) bg_fetcher.tile_number + offset + 1);
        bg_fetcher.mode = PUSH;
        break;
    case PUSH:
        if (pixel_mixer.bg_pixels_len == 0) {
            for (int i = 7; i >= 0; i--) {
                pixel_mixer.bg_pixels[i] = (((bg_fetcher.data_high & (1 << i)) >> i) << 1) |
                                            ((bg_fetcher.data_low  & (1 << i)) >> i);
            }
            pixel_mixer.bg_pixels_len = 8;
            bg_fetcher.mode = GET_TILE;
            bg_fetcher.delay = 1;
        }
    }
}

static void
step_mixer()
{
    if (pixel_mixer.warmup && pixel_mixer.warmup--) return;
    if (pixel_mixer.bg_pixels_len == 0) return;
    if (pixel_mixer.discard && pixel_mixer.discard-- && pixel_mixer.bg_pixels_len--) return;

    uint16_t lcd_index = ppu.LY * LCD_WIDTH + ppu.lx;
    ppu.lcd[lcd_index] = LCD_COLORS[(ppu.BGP >> (2 * pixel_mixer.bg_pixels[--pixel_mixer.bg_pixels_len])) &0x03];
}

static void
mode3(void)
{
    step_bg_fetcher();
}

void
dot_cycle(void)
{
    switch (PPU_MODE) {
    case PPU_MODE_OAM_SCAN:
        if (ppu.frame_dot % 2 == 0 && ppu.oam_scan_count < SCANLINE_MAX_OBJS)
            oam_scan_step();
        // last dot of oam scan, prepare to enter drawing mode next dot
        if (ppu.frame_dot % DOTS_PER_SCANLINE == 79)
            begin_draw_mode();
        break;
    case PPU_MODE_DRAWING:
        if (obj_fetcher.mode != IDLE)
            step_obj_fetcher();
        if (obj_fetcher.mode == IDLE) {
            step_bg_fetcher();
            step_mixer();
            obj_fetcher.mode = CHECK_X;
            if (++ppu.lx == LCD_WIDTH)
                PPU_MODE_SET(PPU_MODE_HBLANK);
        }
        break;
    case PPU_MODE_HBLANK:
    case PPU_MODE_VBLANK:
    }

    ppu.frame_dot++;
    if (ppu.frame_dot % DOTS_PER_SCANLINE == 0) {
        ppu.LY = (ppu.LY + 1) % SCANLINES_PER_FRAME;
        check_lyc_interrupt();
        if (ppu.LY >= SCANLINES_PER_FRAME) {
            // finished a frame
            ppu.LY = 0;
            ppu.frame_dot = 0;
            ppu.window_condition = false;
        }

        if (ppu.LY == LCD_HEIGHT) {
            PPU_MODE_SET(PPU_MODE_VBLANK);
            bg_fetcher.win_line_counter = 0;
            ppu.irq->IF |= 1;
            // do i need vblank interrupts? will do later
        } else if (ppu.LY < LCD_HEIGHT) {
            PPU_MODE_SET(PPU_MODE_OAM_SCAN);
        }
        ppu.oam_scan_count = 0;
    }
}

uint32_t *
get_lcd(void)
{
    return ppu.lcd;
}

gb_ppu_t *
init_gameboy_ppu(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    ppu.irq = irq;
    bus->interface_vram = &bus_vram;
    bus->interface_oam = &bus_oam;
    bus->interface_registers_ppu = &bus_registers_ppu;
    return &ppu;
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