#include "_abi.h"
#include "ppu.h"

// stuff with PPU addressable registers
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
#define READONLY_STAT               0b00000111

#define STAT_BIT_LYC_INT_SELECT     0b01000000
#define STAT_BIT_MD2_INT_SELECT     0b00100000
#define STAT_BIT_MD1_INT_SELECT     0b00010000
#define STAT_BIT_MD0_INT_SELECT     0b00001000
#define STAT_BIT_EQL                0b00000100

#define LCDC_ENABLE_WINDOW          (ppu.LCDC & LCDC_BIT_WIN_ENABLE)
#define LCDC_ENABLE_OBJ             (ppu.LCDC & LCDC_BIT_OBJ_ENABLE)
#define LCDC_ENABLE_PRIORITY        (ppu.LCDC & LCDC_BIT_BG_WIN_PRIORITY)

#define STAT_LYC_INT_SELECT         (ppu.STAT & STAT_BIT_LYC_INT_SELECT)
#define STAT_MD2_INT_SELECT         (ppu.STAT & STAT_BIT_MD2_INT_SELECT)
#define STAT_MD1_INT_SELECT         (ppu.STAT & STAT_BIT_MD1_INT_SELECT)
#define STAT_MD0_INT_SELECT         (ppu.STAT & STAT_BIT_MD0_INT_SELECT)
#define STAT_EQL                    (ppu.STAT & STAT_BIT_EQL)

// 8px or 16px sprite heights
#define OBJ_HEIGHT                  ((((ppu.LCDC & LCDC_BIT_OBJ_HEIGHT) >> 2) + 1) * 8)
#define WIN_TILE_MAP                (((ppu.LCDC & LCDC_BIT_WIN_TILEMAP) >> 6) ? 0x9C00 : 0x9800)
#define BG_TILE_MAP                 (((ppu.LCDC & LCDC_BIT_BG_TILE_MAP) >> 3) ? 0x9C00 : 0x9800)
#define BG_TILE_DATA_METHOD         ((ppu.LCDC & LCDC_BIT_BG_WIN_TILES) >> 4)

#define PPU_MODE_MASK               0b00000011
#define PPU_MODE                    (ppu.STAT & PPU_MODE_MASK)
#define PPU_ENABLED                 (ppu.LCDC & LCDC_BIT_PPU_ENABLE)

// macros regarding object attribute memory and an object's flag byte
#define OAM_FLAG_BIT_PRIORITY       0b10000000
#define OAM_FLAG_BIT_YFLIP          0b01000000
#define OAM_FLAG_BIT_XFLIP          0b00100000
#define OAM_FLAG_BIT_PALETTE        0b00010000

#define OAM_Y(i)                    (ppu.oam[(i) * 4])
#define OAM_X(i)                    (ppu.oam[(i) * 4 + 1])
#define OAM_TILE_INDEX(i)           (ppu.oam[(i) * 4 + 2])
#define OAM_FLAGS(i)                (ppu.oam[(i) * 4 + 3])

#define OAM_PRIORITY(i)             ((OAM_FLAGS(i) & OAM_FLAG_BIT_PRIORITY) >> 7)
#define OAM_YFLIP(i)                ((OAM_FLAGS(i) & OAM_FLAG_BIT_YFLIP) >> 6)
#define OAM_XFLIP(i)                ((OAM_FLAGS(i) & OAM_FLAG_BIT_XFLIP) >> 5)
#define OAM_PALETTE(i)              ((OAM_FLAGS(i) & OAM_FLAG_BIT_PALETTE) >> 4)

#define VRAM(address)               (ppu.vram[(address) - ADDR_START_VRAM])
#define  OAM(address)               (ppu.oam [((address) - ADDR_START_OAM_MEM) % GB_DMG_OAM_SIZE])

// mix bit i of the high and low bytes into a 2 bit palette index
#define MIX_HIGH_LOW(high, low, i)  ((((high & (1 << i)) >> i) << 1) | ((low & (1 << i)) >> i))

#define TILEMAP_W                   32
#define LOW                         0
#define HIGH                        1

typedef enum {
    PPU_MODE_HBLANK,
    PPU_MODE_VBLANK,
    PPU_MODE_OAM_SCAN,
    PPU_MODE_DRAWING
} ppu_mode_t;

// for current status of background and object fetchers
typedef enum {
    BG_GET_TILE,
    BG_GET_DATA_LOW,
    BG_GET_DATA_HIGH,
    BG_PUSH,
} bg_fetcher_mode_t;

typedef enum {
    OBJ_GET_TILE,
    OBJ_GET_DATA_LOW,
    OBJ_GET_DATA_HIGH,
    OBJ_CHECK_X,
    OBJ_IDLE
} obj_fetcher_mode_t;

// choose between the two addressing modes for interpreting tile data indices
typedef enum {
    TILE_METHOD_8800,
    TILE_METHOD_8000
} tile_data_addressing_mode_t;

typedef struct {
    uint8_t color;
    uint8_t palette;
    uint8_t bg_priority;
} obj_pixel_t;

static struct {
    bg_fetcher_mode_t mode;
    uint8_t delay;
    uint8_t data_low;
    uint8_t data_high;
    uint8_t tile_data_index;

    bool window_active;
    bool window_condition;
    uint16_t tile_map_index;
    uint8_t window_line;
} bg_fetcher;

static struct {
    obj_fetcher_mode_t mode;
    uint8_t delay;
    uint8_t data_low;
    uint8_t data_high;
    uint16_t tile_data_index;
    
    uint8_t buf_index;
    bool index_done[10];
} obj_fetcher;

static struct {
    uint8_t discard;

    obj_pixel_t obj_pixels[8];
    uint8_t obj_pixels_index;
    uint8_t bg_pixels[8];
    uint8_t bg_pixels_len;
} pixel_mixer;

// Ordered as ARGB as written, but as BGRA as little endian
static const uint32_t DEFAULT_LCD_COLORS[4] = {
    0xFFF4FFBF,
    0xFF9AD941,
    0xFF608057,
    0xFF002B40
};

static gb_ppu_t ppu;

static void 
update_stat_interrupt(void)
{
    if (PPU_ENABLED) {
        ppu.irq->update_stat_interrupt_line(
            (STAT_LYC_INT_SELECT &&  STAT_EQL                      ) ||
            (STAT_MD0_INT_SELECT && (PPU_MODE == PPU_MODE_HBLANK  )) ||
            (STAT_MD1_INT_SELECT && (PPU_MODE == PPU_MODE_VBLANK  )) ||
            (STAT_MD2_INT_SELECT && (PPU_MODE == PPU_MODE_OAM_SCAN))
        );
    }
}

// set bit 2 of STAT register based on LYC == LY comparison, return true if a CHANGE occurred
static void
stat_compare_ly_lyc(void)
{
    uint8_t old_val = STAT_EQL;
    ppu.STAT = (ppu.STAT & (STAT_BIT_EQL ^ 0xFF)) | ((ppu.LY == ppu.LYC) << 2);
    if (old_val != STAT_EQL)
        update_stat_interrupt();
}

static void
ppu_set_mode(ppu_mode_t mode)
{
    ppu.STAT = (ppu.STAT & (PPU_MODE_MASK ^ 0xFF)) | mode;
    update_stat_interrupt();
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
    default:                        return UNREADABLE;
    }       
}
static void
write_ppu_reg(memaddr address, uint8_t val)
{
    switch (address) {
    case MEMADDR_LCDC:
        uint8_t ppu_enabled_before = PPU_ENABLED;
        ppu.LCDC   = val;

        if (ppu_enabled_before && !PPU_ENABLED) {
            ppu.irq->update_stat_interrupt_line(0);
            ppu_set_mode(PPU_MODE_HBLANK);
            ppu.LY = 0;
            ppu.line_dot = 0;
            ppu.lcd.blank_frames = 1;
        } else if (!ppu_enabled_before && PPU_ENABLED) {
            stat_compare_ly_lyc();
            ppu_set_mode(PPU_MODE_OAM_SCAN);
        }
        break;
    case MEMADDR_STAT:              ppu.STAT   = (WRITEABLE_STAT & val) | (READONLY_STAT & ppu.STAT); break;
    case MEMADDR_SCY:               ppu.SCY    = val;                                                 break;
    case MEMADDR_SCX:               ppu.SCX    = val;                                                 break;
    case MEMADDR_LY:                                                                                  break;                                 
    case MEMADDR_LYC:               ppu.LYC    = val; stat_compare_ly_lyc();                          break;
    case MEMADDR_BGP:               ppu.BGP    = val;                                                 break;
    case MEMADDR_OBP0:              ppu.OBP[0] = val;                                                 break;
    case MEMADDR_OBP1:              ppu.OBP[1] = val;                                                 break;
    case MEMADDR_WY:                ppu.WY     = val;                                                 break;
    case MEMADDR_WX:                ppu.WX     = val;                                                 break;
    }
}
static bus_interface_t bus_registers_ppu = { .read = read_ppu_reg, .write = write_ppu_reg };

static uint8_t read_vram(memaddr address) {
    return VRAM(address);
}
static void write_vram(memaddr address, uint8_t val) {
    VRAM(address) = val;
}
static bus_interface_t bus_vram = { .read = read_vram, .write = write_vram };

static uint8_t read_oam(memaddr address) {
    return OAM(address);
}
static void write_oam(memaddr address, uint8_t val) {
    OAM(address) = val;
}
static bus_interface_t bus_oam = { .read = read_oam, .write = write_oam };

// Check whether BG fetcher should transition to window mode
static inline bool check_window_ready(void) {
    return bg_fetcher.window_condition && LCDC_ENABLE_WINDOW && ppu.lx + 7 >= ppu.WX + 8;
}

// Get y offset of currently processed sprite object to decide which row of data to load, checking Y-flip
static inline uint8_t
obj_y_offset(void)
{
    uint8_t sprite_height = OBJ_HEIGHT;
    uint8_t y_offset = (ppu.LY - OAM_Y(ppu.obj_buffer[obj_fetcher.buf_index]) + 16) % sprite_height;
    if (OAM_YFLIP(ppu.obj_buffer[obj_fetcher.buf_index]))
        y_offset = sprite_height - 1 - y_offset;
    return y_offset;
}

// Returns a low or high byte of tile data using some tile index, some y row, using particular addressing mode 
static uint8_t
tile_data(uint8_t byte, tile_data_addressing_mode_t addressing, uint8_t tile_data_index, uint8_t y_offset)
{
    if (addressing == TILE_METHOD_8000)
        return VRAM(ADDR_TILE_DATA_BLOCK_0 + 16 * tile_data_index + 2 * y_offset + byte);
    else
        return VRAM(ADDR_TILE_DATA_BLOCK_2 + 16 * (int8_t) tile_data_index + 2 * y_offset + byte);
}

// Push object fetcher's data into the pixel mixer obj fifo
static void
push_obj_pixels(void)
{
    uint8_t oam_index = ppu.obj_buffer[obj_fetcher.buf_index];
    for (int i = 7; i >= 0; i--) {
        uint8_t idx = ((OAM_XFLIP(oam_index) ? i : 7 - i) + pixel_mixer.obj_pixels_index) % 8;
        // Ignore this pixel if the obj pixel FIFO already has a non-transparent pixel
        if (pixel_mixer.obj_pixels[idx].color)
            continue;
        pixel_mixer.obj_pixels[idx].color = MIX_HIGH_LOW(obj_fetcher.data_high, obj_fetcher.data_low, i);
        pixel_mixer.obj_pixels[idx].palette = OAM_PALETTE(oam_index);
        pixel_mixer.obj_pixels[idx].bg_priority = OAM_PRIORITY(oam_index);
    }
}

// Investigate next obj in OAM to see if it should be added to obj buffer for this scanline
static void
oam_scan_step(void)
{
    uint8_t oam_index = ppu.line_dot / 2;
    // dont add objs with X 0, or do? references seem to disagree on this point TODO
    // if (OAM_X(oam_index) == 0) return;
    if (OAM_Y(oam_index) <= ppu.LY + 16 && ppu.LY + 16 < OAM_Y(oam_index) + OBJ_HEIGHT)
        ppu.obj_buffer[ppu.obj_buffer_size++] = oam_index;
}

static void
transition_draw_mode(void)
{
    // start 8 off-screen, helps with processing sprites that start off-screen
    ppu.lx = 0;

    obj_fetcher.mode = OBJ_CHECK_X;
    obj_fetcher.buf_index = 0;
    for (int i = 0; i < SCANLINE_MAX_OBJS; i++)
        obj_fetcher.index_done[i] = (i >= ppu.obj_buffer_size);

    bg_fetcher.mode = BG_GET_TILE;
    bg_fetcher.delay = 5;
    bg_fetcher.window_active = false;
    if (bg_fetcher.window_active)
        bg_fetcher.tile_map_index = TILEMAP_W * (bg_fetcher.window_line / 8);
    else
        bg_fetcher.tile_map_index = TILEMAP_W * (((ppu.LY + ppu.SCY) % 256) / 8) + ppu.SCX / 8;

    // delay of 12 at start of scanline comes from 4 + outputting 8 garbage pixels off-screen
    obj_fetcher.delay = 4;
    pixel_mixer.bg_pixels_len = 8;
    memset(pixel_mixer.obj_pixels, 0, sizeof(pixel_mixer.obj_pixels));
}

static void
step_obj_fetcher(void)
{
    if (obj_fetcher.delay && obj_fetcher.delay--) return;
    switch (obj_fetcher.mode) {

    case OBJ_GET_TILE:
        obj_fetcher.tile_data_index = OAM_TILE_INDEX(ppu.obj_buffer[obj_fetcher.buf_index]);
        if (OBJ_HEIGHT == 16)
            obj_fetcher.tile_data_index &= 0xFE;
        obj_fetcher.mode = OBJ_GET_DATA_LOW;
        obj_fetcher.delay = 1;
        break;

    case OBJ_GET_DATA_LOW:
        obj_fetcher.data_low = tile_data(
            LOW, TILE_METHOD_8000, obj_fetcher.tile_data_index, obj_y_offset()
        );
        obj_fetcher.mode = OBJ_GET_DATA_HIGH;
        obj_fetcher.delay = 1;
        break;

    case OBJ_GET_DATA_HIGH:
        obj_fetcher.data_high = tile_data(
            HIGH, TILE_METHOD_8000, obj_fetcher.tile_data_index, obj_y_offset()
        );

        push_obj_pixels();
        obj_fetcher.buf_index++;
        obj_fetcher.mode = OBJ_CHECK_X;
        // attempt to create the post-sprite fetch delay in bg fetcher, unsure of this
        switch (bg_fetcher.mode) {
        case BG_GET_TILE:                       bg_fetcher.delay = 1; break;
        case BG_GET_DATA_LOW:                   bg_fetcher.delay = 3; break;
        case BG_GET_DATA_HIGH:                  bg_fetcher.delay = 5; break;
        case BG_PUSH:                           bg_fetcher.delay = 6; break;
        }
        break;

    case OBJ_CHECK_X:
        for (int i = obj_fetcher.buf_index; i < SCANLINE_MAX_OBJS; i++, obj_fetcher.buf_index++) {
            if (obj_fetcher.index_done[i]) continue;
            if (OAM_X(ppu.obj_buffer[i]) <= (ppu.lx + 8) - 8) {
                obj_fetcher.mode = OBJ_GET_TILE;
                obj_fetcher.index_done[i] = true;
                break;
            }
        }
        if (obj_fetcher.buf_index == SCANLINE_MAX_OBJS)
            obj_fetcher.mode = OBJ_IDLE;
        break;
    case OBJ_IDLE:
    }
}

static void
step_bg_fetcher(void)
{
    uint8_t y_offset;

    if (bg_fetcher.delay && bg_fetcher.delay--) return;
    switch (bg_fetcher.mode) {
        
    case BG_GET_TILE:
        bg_fetcher.tile_data_index = VRAM(
            (bg_fetcher.window_active ? WIN_TILE_MAP : BG_TILE_MAP) + bg_fetcher.tile_map_index
        );
        // increment tile index but wrap to start of same row if past column 31
        if (++bg_fetcher.tile_map_index % TILEMAP_W == 0) bg_fetcher.tile_map_index -= TILEMAP_W;
        bg_fetcher.mode = BG_GET_DATA_LOW;
        bg_fetcher.delay = 1;
        break;

    case BG_GET_DATA_LOW:
        y_offset = bg_fetcher.window_active ? (bg_fetcher.window_line % 8) : ((ppu.LY + ppu.SCY) % 8);
        bg_fetcher.data_low = tile_data(
            LOW, BG_TILE_DATA_METHOD, bg_fetcher.tile_data_index, y_offset
        );
        bg_fetcher.mode = BG_GET_DATA_HIGH;
        bg_fetcher.delay = 1;
        break;

    case BG_GET_DATA_HIGH:
        y_offset = bg_fetcher.window_active ? (bg_fetcher.window_line % 8) : ((ppu.LY + ppu.SCY) % 8);
        bg_fetcher.data_high = tile_data(
            HIGH, BG_TILE_DATA_METHOD, bg_fetcher.tile_data_index, y_offset
        );
        bg_fetcher.mode = BG_PUSH;
        break;

    case BG_PUSH:
        if (pixel_mixer.bg_pixels_len == 0) {
            for (int i = 7; i >= 0; i--)
                pixel_mixer.bg_pixels[i] = MIX_HIGH_LOW(bg_fetcher.data_high, bg_fetcher.data_low, i);
            pixel_mixer.bg_pixels_len = 8;
            bg_fetcher.mode = BG_GET_TILE;
            bg_fetcher.delay = 1;
        }
        break;
    }
}

static void
step_mixer()
{
    if (pixel_mixer.bg_pixels_len == 0) return;
    if (pixel_mixer.discard && pixel_mixer.discard-- && pixel_mixer.bg_pixels_len--) return;

    obj_pixel_t pixel = pixel_mixer.obj_pixels[pixel_mixer.obj_pixels_index];
    uint8_t bg_color_index = pixel_mixer.bg_pixels[pixel_mixer.bg_pixels_len - 1];
    uint8_t color = 0b00;

    if (LCDC_ENABLE_PRIORITY && (!LCDC_ENABLE_OBJ || !pixel.color || (pixel.bg_priority && bg_color_index)))
        color = (ppu.BGP >> (2 * bg_color_index)) & 0b11;
    else if (LCDC_ENABLE_OBJ)
        color = (ppu.OBP[pixel.palette] >> (2 * pixel.color)) & 0b11;

    // increment ppu.lx here, screen starts at ppu.lx value of 8, do BG discard there
    if (ppu.lx >= 8)
        ppu.lcd.pixels[ppu.LY * LCD_WIDTH + (ppu.lx - 8)] = ppu.lcd.colors[color];
    if (++ppu.lx == 8)
        pixel_mixer.discard = ppu.SCX % 8;

    if (!bg_fetcher.window_active && check_window_ready()) {
        pixel_mixer.discard = 0;
        bg_fetcher.window_active = true;
        bg_fetcher.tile_map_index = TILEMAP_W * (bg_fetcher.window_line / 8);
        pixel_mixer.bg_pixels_len = 0;
        bg_fetcher.mode = BG_GET_TILE;
        bg_fetcher.delay = 1;
    } else
        pixel_mixer.bg_pixels_len--;
        
    pixel_mixer.obj_pixels[pixel_mixer.obj_pixels_index++].color = 0;
    pixel_mixer.obj_pixels_index %= 8;
    obj_fetcher.mode = OBJ_CHECK_X;
    obj_fetcher.buf_index = 0;
}

bool
cycle_tcycle_ppu(void)
{
    if (!PPU_ENABLED) return false;
    switch (PPU_MODE) {
    case PPU_MODE_OAM_SCAN:
        if (ppu.line_dot % 2 == 0 && ppu.obj_buffer_size < SCANLINE_MAX_OBJS)
            oam_scan_step();
        if (ppu.line_dot == SCANLINE_FINAL_OAM_SCAN_DOT) {
            transition_draw_mode();
            ppu_set_mode(PPU_MODE_DRAWING);
        }
        break;
    case PPU_MODE_DRAWING:
        if (obj_fetcher.mode != OBJ_IDLE)
            step_obj_fetcher();
        if (obj_fetcher.mode == OBJ_IDLE) {
            step_bg_fetcher();
            step_mixer();
            if (ppu.lx == LCD_WIDTH + 8)
                ppu_set_mode(PPU_MODE_HBLANK);
        }
        break;
    case PPU_MODE_HBLANK:
    case PPU_MODE_VBLANK:
    }

    // Check whether scanline finished
    if (ppu.line_dot++ == SCANLINE_FINAL_DOT) {
        if (bg_fetcher.window_active)
            bg_fetcher.window_line++;

        // Check whether frame finished
        if (++ppu.LY == SCANLINES_PER_FRAME) {
            ppu.LY = 0;
            bg_fetcher.window_condition = false;
            bg_fetcher.window_line = 0;
            ppu.lcd.blank_frames = 0;
        }
        stat_compare_ly_lyc();

        ppu.line_dot = 0;
        ppu.obj_buffer_size = 0;

        if (ppu.LY < LCD_HEIGHT) {
            ppu_set_mode(PPU_MODE_OAM_SCAN);
            if (ppu.LY == ppu.WY)
                bg_fetcher.window_condition = true;
        } else if (ppu.LY == LCD_HEIGHT) {
            ppu_set_mode(PPU_MODE_VBLANK);
            // signal a vblank transition to both interrupts and emulation
            ppu.irq->IF |= INTERRUPT_BIT_VBLANK;
            return true;
        }
    }
    return false;
}

gb_ppu_t *
init_gameboy_ppu(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    memset(&ppu, 0, sizeof(gb_ppu_t));
    ppu.irq = irq;
    GB_set_lcd_colors((uint32_t *) DEFAULT_LCD_COLORS);
    bus->interface_vram = &bus_vram;
    bus->interface_oam = &bus_oam;
    bus->interface_reg_ppu = &bus_registers_ppu;
    return &ppu;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions
        
###############################################################################
############################################################################ */

uint32_t *GB_get_lcd(void) {
    return ppu.lcd.pixels;
}

gb_return_t GB_set_lcd_colors(uint32_t colors[4]) {
    memcpy(ppu.lcd.colors, colors, 4 * sizeof(uint32_t));
    return GB_RETURN_OK;
}