#include "ppu.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LCDC_BIT_PPU_ENABLE         0b10000000
#define LCDC_BIT_WIN_TILEMAP        0b01000000
#define LCDC_BIT_WIN_ENABLE         0b00100000
#define LCDC_BIT_BG_WIN_TILES       0b00010000
#define LCDC_BIT_BG_TILE_MAP        0b00001000
#define LCDC_BIT_OBJ_HEIGHT         0b00000100
#define LCDC_BIT_OBJ_ENABLE         0b00000010
#define LCDC_BIT_BG_WIN_PRIORITY    0b00000001

#define USEPINS_STAT                0b01111111
#define WRITEABLE_STAT              0b01111000

#define STAT_BIT_LYC_INT_SELECT     0b01000000
#define STAT_BIT_MD2_INT_SELECT     0b00100000
#define STAT_BIT_MD1_INT_SELECT     0b00010000
#define STAT_BIT_MD0_INT_SELECT     0b00001000
#define STAT_BIT_EQL                0b00000100

#define OAM_FLAG_BIT_PRIORITY       0b10000000
#define OAM_FLAG_BIT_YFLIP          0b01000000
#define OAM_FLAG_BIT_XFLIP          0b00100000
#define OAM_FLAG_BIT_PALETTE        0b00010000

#define LCDC_ENABLE_WINDOW          (ppu.LCDC & LCDC_BIT_WIN_ENABLE)
#define LCDC_ENABLE_OBJ             (ppu.LCDC & LCDC_BIT_OBJ_ENABLE)
#define LCDC_ENABLE_PRIORITY        (ppu.LCDC & LCDC_BIT_BG_WIN_PRIORITY)

#define BG_TILE_DATA_METHOD_8000    0b00000001
#define BG_TILE_DATA_METHOD_8800    0b00000000
// 8px or 16px sprite heights
#define OBJ_HEIGHT                  ((((ppu.LCDC & LCDC_BIT_OBJ_HEIGHT) >> 2) + 1) * 8)
#define WIN_TILE_MAP                (((ppu.LCDC & LCDC_BIT_WIN_TILEMAP) >> 6) ? 0x9C00 : 0x9800)
#define BG_TILE_MAP                 (((ppu.LCDC & LCDC_BIT_BG_TILE_MAP) >> 3) ? 0x9C00 : 0x9800)
#define BG_TILE_DATA_METHOD         ((ppu.LCDC & LCDC_BIT_BG_WIN_TILES) >> 4)

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

#define VRAM(address)               (ppu.vram[(address) - ADDR_START_VRAM])
#define  OAM(address)               (ppu.oam [(address) - ADDR_START_OAM_MEM])

typedef enum {
    GET_TILE,
    GET_DATA_LOW,
    GET_DATA_HIGH,
    PUSH,
    CHECK_X,
    IDLE
} fetcher_mode_t;

typedef struct {
    uint8_t color;
    uint8_t palette;
    uint8_t bg_priority;
} obj_pixel_t;

static struct {
    fetcher_mode_t mode;
    bool window_active;
    bool window_condition;
    uint8_t delay;
    uint8_t data_low;
    uint8_t data_high;

    uint16_t tile;
    uint8_t tile_number;
    uint8_t window_line;
} bg_fetcher;

static struct {
    fetcher_mode_t mode;
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

    obj_pixel_t obj_pixels[8];
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
    case MEMADDR_LY:                return ppu.LY;
    case MEMADDR_LYC:               return ppu.LYC; 
    case MEMADDR_BGP:               return ppu.BGP;
    case MEMADDR_OBP0:              return ppu.OBP[0];
    case MEMADDR_OBP1:              return ppu.OBP[1];
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
    case MEMADDR_OBP0:              ppu.OBP[0] = val;                                               break;
    case MEMADDR_OBP1:              ppu.OBP[1] = val;                                               break;
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
    uint8_t obj_index = ppu.line_dot/ 2;
    // dont add objs with X 0
    if (OAM_X(obj_index) == 0) 
        return;      
    if (OAM_Y(obj_index) <= ppu.LY + 16 && ppu.LY + 16 < OAM_Y(obj_index) + OBJ_HEIGHT)
        ppu.obj_buffer[ppu.obj_buffer_size++] = obj_index;
}

static void
transition_draw_mode(void)
{
    ppu.lx = 0;

    obj_fetcher.mode = CHECK_X;
    obj_fetcher.searched = 0;
    for (int i = 0; i < SCANLINE_MAX_OBJS; i++)
        obj_fetcher.index_used[i] = (i >= ppu.obj_buffer_size);

    bg_fetcher.mode = GET_TILE;
    bg_fetcher.delay = 5;
    bg_fetcher.window_active = false;
    if (!bg_fetcher.window_active && bg_fetcher.window_condition && LCDC_ENABLE_WINDOW && ppu.lx + 7 >= ppu.WX) {
        bg_fetcher.window_active = true;
        bg_fetcher.tile = 32 * (bg_fetcher.window_line / 8);
        bg_fetcher.delay = 5;       // only thing changed from other spot, these really need to be combined
    } else {
        bg_fetcher.tile = (((ppu.SCX / 8) & 0x1F) + 32 * (((ppu.LY + ppu.SCY) & 0xFF) / 8)) & 0x03FF;
        pixel_mixer.discard = ppu.SCX % 8;
    }

    pixel_mixer.bg_pixels_len = 0;
    memset(pixel_mixer.obj_pixels, 0, sizeof(pixel_mixer.obj_pixels));
    pixel_mixer.warmup = 12;
}

static void
step_obj_fetcher(void)
{
    if (obj_fetcher.delay && obj_fetcher.delay--) return;
    switch (obj_fetcher.mode) {
    case GET_TILE:
        obj_fetcher.tile_number = OAM_TILE_INDEX(ppu.obj_buffer[obj_fetcher.searched]);
        if (OBJ_HEIGHT == 16)
            obj_fetcher.tile_number &= 0xFE;
        obj_fetcher.mode = GET_DATA_LOW;
        obj_fetcher.delay = 1;
        break;
    case GET_DATA_LOW:
        uint8_t y_offset = (ppu.LY - OAM_Y(ppu.obj_buffer[obj_fetcher.searched]) + 16) % OBJ_HEIGHT;
        if (OAM_YFLIP(ppu.obj_buffer[obj_fetcher.searched]))
            y_offset = OBJ_HEIGHT - 1 - y_offset;
        obj_fetcher.data_low = VRAM(0x8000 + 16 * obj_fetcher.tile_number + 2 * y_offset);
        obj_fetcher.mode = GET_DATA_HIGH;
        obj_fetcher.delay = 1;
        break;
    case GET_DATA_HIGH:
        uint8_t offset = (ppu.LY - OAM_Y(ppu.obj_buffer[obj_fetcher.searched]) + 16) % OBJ_HEIGHT;
        if (OAM_YFLIP(ppu.obj_buffer[obj_fetcher.searched]))
            offset = OBJ_HEIGHT - 1 - offset;
        obj_fetcher.data_high = VRAM(0x8000 + 16 * obj_fetcher.tile_number + 2 * offset + 1);

        // xflip here?
        for (int i = 7; i >= 0; i--) {
            uint8_t idx = OAM_XFLIP(ppu.obj_buffer[obj_fetcher.searched]) ? i : 7 - i;
            if (pixel_mixer.obj_pixels[idx].color) continue;
            pixel_mixer.obj_pixels[idx].color = (((obj_fetcher.data_high & (1 << i)) >> i) << 1) |
                                                   ((obj_fetcher.data_low  & (1 << i)) >> i);
            pixel_mixer.obj_pixels[idx].bg_priority = OAM_PRIORITY(ppu.obj_buffer[obj_fetcher.searched]);
            pixel_mixer.obj_pixels[idx].palette = OAM_PALETTE(ppu.obj_buffer[obj_fetcher.searched]);
        }
        //obj_fetcher.searched++;
        obj_fetcher.mode = CHECK_X;
        break;
    case CHECK_X:
        for (int i = obj_fetcher.searched; i < SCANLINE_MAX_OBJS; i++, obj_fetcher.searched++) {
            if (obj_fetcher.index_used[i]) continue;
            if (OAM_X(ppu.obj_buffer[i]) <= ppu.lx + 8) {
                obj_fetcher.mode = GET_TILE;
                // obj_fetcher.delay = 1; // no delay i think already built in by 1 dot on check mode?
                obj_fetcher.index_used[i] = true;
                break;
            }
        }
        if (obj_fetcher.searched == SCANLINE_MAX_OBJS)
            // TODO reset bg fetcher to get tile mode?
            obj_fetcher.mode = IDLE;
        break;
    }
}

static void
step_bg_fetcher(void)
{
    uint8_t y_offset;
    if (bg_fetcher.delay && bg_fetcher.delay--) return;
    switch (bg_fetcher.mode) {
    case GET_TILE:
        if (bg_fetcher.window_active)
            bg_fetcher.tile_number = VRAM(WIN_TILE_MAP + bg_fetcher.tile);
        else
            bg_fetcher.tile_number = VRAM(BG_TILE_MAP + bg_fetcher.tile);
        if (bg_fetcher.tile % 32 == 31)
            bg_fetcher.tile -= 31;
        else
            bg_fetcher.tile++;
        bg_fetcher.mode = GET_DATA_LOW;
        bg_fetcher.delay = 1;
        break;
    case GET_DATA_LOW:
        if (bg_fetcher.window_active)
            y_offset = 2 * (bg_fetcher.window_line % 8);
        else
            y_offset = 2 * ((ppu.LY + ppu.SCY) % 8);
        if (BG_TILE_DATA_METHOD == BG_TILE_DATA_METHOD_8000)
            bg_fetcher.data_low = VRAM(0x8000 + 16 * bg_fetcher.tile_number + y_offset);
        else
            bg_fetcher.data_low = VRAM(0x9000 + 16 * (int8_t) bg_fetcher.tile_number + y_offset);
        bg_fetcher.mode = GET_DATA_HIGH;
        bg_fetcher.delay = 1;
        break;
    case GET_DATA_HIGH:
        // TODO fix the ugliness later
        if (bg_fetcher.window_active)
            y_offset = 2 * (bg_fetcher.window_line % 8);
        else
            y_offset = 2 * ((ppu.LY + ppu.SCY) % 8);
        if (BG_TILE_DATA_METHOD == BG_TILE_DATA_METHOD_8000)
            bg_fetcher.data_high = VRAM(0x8000 + 16 * bg_fetcher.tile_number + y_offset + 1);
        else
            bg_fetcher.data_high = VRAM(0x9000 + 16 * (int8_t) bg_fetcher.tile_number + y_offset + 1);
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

    obj_pixel_t pixel = pixel_mixer.obj_pixels[0];
    uint8_t bg_color_index = pixel_mixer.bg_pixels[pixel_mixer.bg_pixels_len - 1];
    uint8_t color = 0b00;
    if (LCDC_ENABLE_PRIORITY && (!LCDC_ENABLE_OBJ || !pixel.color || (pixel.bg_priority && bg_color_index)))
        color = ppu.BGP >> (2 * bg_color_index);
    else if (LCDC_ENABLE_OBJ)
        color = ppu.OBP[pixel.palette] >> (2 * pixel.color);

    // this is where we increment ppu.lx
    uint16_t lcd_index = ppu.LY * LCD_WIDTH + ppu.lx++;
    ppu.lcd[lcd_index] = LCD_COLORS[color & 0x03];

    if (!bg_fetcher.window_active && bg_fetcher.window_condition && LCDC_ENABLE_WINDOW && ppu.lx + 7 >= ppu.WX) {
        bg_fetcher.window_active = true;
        bg_fetcher.tile = 32 * (bg_fetcher.window_line / 8);
        pixel_mixer.bg_pixels_len = 0;
        bg_fetcher.mode = GET_TILE;
        bg_fetcher.delay = 1;
    } else
        pixel_mixer.bg_pixels_len--;

    memmove(pixel_mixer.obj_pixels, pixel_mixer.obj_pixels + 1, 7 * sizeof(obj_pixel_t));
    memset(pixel_mixer.obj_pixels + 7, 0, sizeof(obj_pixel_t));
    obj_fetcher.mode = CHECK_X;
    obj_fetcher.searched = 0;
}

void
dot_cycle(void)
{
    switch (PPU_MODE) {
    case PPU_MODE_OAM_SCAN:
        if (ppu.line_dot % 2 == 0 && ppu.obj_buffer_size < SCANLINE_MAX_OBJS)
            oam_scan_step();
        if (ppu.line_dot == SCANLINE_FINAL_OAM_SCAN_DOT) {
            transition_draw_mode();
            PPU_MODE_SET(PPU_MODE_DRAWING);
        }
        break;
    case PPU_MODE_DRAWING:
        if (obj_fetcher.mode != IDLE)
            step_obj_fetcher();
        if (obj_fetcher.mode == IDLE) {
            step_bg_fetcher();
            step_mixer();
            if (ppu.lx == LCD_WIDTH)
                PPU_MODE_SET(PPU_MODE_HBLANK);
        }
        break;
    case PPU_MODE_HBLANK:
    case PPU_MODE_VBLANK:
    }

    // Check whether scanline finished
    if (++ppu.line_dot == SCANLINE_FINAL_DOT) {
        if (bg_fetcher.window_active)
            bg_fetcher.window_line++;

        // Check whether frame finished
        if (++ppu.LY == SCANLINES_PER_FRAME) {
            ppu.LY = 0;
            bg_fetcher.window_condition = false;
            bg_fetcher.window_line = 0;
        }
        check_lyc_interrupt();

        if (ppu.LY < LCD_HEIGHT) {
            PPU_MODE_SET(PPU_MODE_OAM_SCAN);
            if (ppu.LY == ppu.WY)
                bg_fetcher.window_condition = true;
        } else if (ppu.LY == LCD_HEIGHT) {
            PPU_MODE_SET(PPU_MODE_VBLANK);
            ppu.irq->IF |= 1;
        }

        ppu.line_dot = 0;
        ppu.obj_buffer_size = 0;
    }
}

uint32_t * get_lcd(void) {
    return ppu.lcd;
}

gb_ppu_t *
init_gameboy_ppu(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    ppu.irq = irq;
    bus->interface_vram = &bus_vram;
    bus->interface_oam = &bus_oam;
    bus->interface_reg_ppu = &bus_registers_ppu;
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