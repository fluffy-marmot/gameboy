#ifndef GB_PPU
#define GB_PPU

#include "_specification.h"
#include "bus.h"
#include "irq.h"

typedef enum {
    SCANLINE_START_DOT = 0,
    SCANLINE_FINAL_OAM_SCAN_DOT = 79,
    SCANLINE_FINAL_DOT = DOTS_PER_SCANLINE - 1
} scanline_dot_t;

typedef struct {
    /* -- addressable registers (not all R/W) and memory -- */
    uint8_t LCDC;                               // LCD control register
    uint8_t STAT;                               // LCD status register
    uint8_t SCY;                                // viewport (scroll) y position
    uint8_t SCX;                                // viewport (scroll) x position
    uint8_t LY;                                 // LCD y coordinate
    uint8_t LYC;                                // LY compare
    
    uint8_t BGP;                                // background (and window) palette 
    uint8_t OBP[2];                             // object palettes OBP0 and OBP1
    uint8_t WY;                                 // window Y
    uint8_t WX;                                 // window X (plus 7)

    uint8_t vram[GB_DMG_VRAM_SIZE];             // video RAM - tile data for 384 tiles and two 32x32 tile maps
    uint8_t  oam[GB_DMG_OAM_SIZE];              // object attribute memory
   
    /* -- PPU internal data -- */
    uint8_t lx;                                 // x coordinate + 8 within scanline (starts off screen)
    scanline_dot_t line_dot;                    // dot number of current scanline (first is 0)
    uint8_t obj_buffer[SCANLINE_MAX_OBJS];      // obj indexes stored by OAM scan
    uint8_t obj_buffer_size;
        
    gb_irq_handler_t *irq;                      // direct access to interrupts data

    /* -- holding actual LCD color data that will be drawn on the screen */
    struct {
        uint32_t pixels[LCD_WIDTH * LCD_HEIGHT];
        uint32_t colors[4];
    } lcd;
} gb_ppu_t;

gb_ppu_t *init_gameboy_ppu(gb_bus_t *, gb_irq_handler_t *);

void cycle_tcycle_ppu(void);

#endif