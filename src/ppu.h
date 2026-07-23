#ifndef GB_PPU
#define GB_PPU

#include "bus.h"
#include "irq.h"

#define LCD_WIDTH                       160
#define LCD_HEIGHT                      144

#define DOTS_PER_SCANLINE               456
#define SCANLINES_PER_FRAME             154

typedef struct {
    uint8_t LCDC;                       // FF40
    uint8_t STAT;                       // FF41
    uint8_t SCY;                        // FF42
    uint8_t SCX;                        // FF43
    uint8_t LY;                         // FF44
    uint8_t LYC;                        // FF45
                                        // FF46 is DMA related - undecided where that belongs
    uint8_t BGP;                        // FF47
    uint8_t OBP0;                       // FF48
    uint8_t OBP1;                       // FF49
    uint8_t WY;                         // FF4A
    uint8_t WX;                         // FF4B

    uint8_t vram[GB_DMG_VRAM_SIZE];
    uint8_t  oam[GB_DMG__OAM_SIZE];
    uint32_t lcd[LCD_WIDTH * LCD_HEIGHT];

    uint16_t frame_dot;


    gb_irq_handler_t *irq;              // direct access to interrupts data
} gb_ppu_t;

gb_ppu_t *init_gameboy_ppu(gb_bus_t *, gb_irq_handler_t *);

#endif