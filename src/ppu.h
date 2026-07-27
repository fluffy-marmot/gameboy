#ifndef GB_PPU
#define GB_PPU

#include "bus.h"
#include "irq.h"
#include "specification.h"

#include <stdbool.h>

typedef struct {
    uint8_t LCDC;
    uint8_t STAT;
    uint8_t SCY;
    uint8_t SCX;
    uint8_t LY;
    uint8_t LYC;
    
    uint8_t BGP;
    uint8_t OBP[2];
    uint8_t WY;
    uint8_t WX;

    uint8_t vram[GB_DMG_VRAM_SIZE];
    uint8_t  oam[GB_DMG_OAM_SIZE];
    uint32_t lcd[LCD_WIDTH * LCD_HEIGHT];

    uint8_t lx;
    uint32_t frame_dot;
    bool window_condition;
    uint8_t oam_scan_indices[SCANLINE_MAX_OBJS];
    uint8_t oam_scan_count;
        
    gb_irq_handler_t *irq;              // direct access to interrupts data
} gb_ppu_t;

gb_ppu_t *init_gameboy_ppu(gb_bus_t *, gb_irq_handler_t *);

// ABI

void dot_cycle(void);
uint32_t *get_lcd(void);
// ABI

#endif