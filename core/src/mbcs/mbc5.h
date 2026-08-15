#ifndef GB_MBC5
#define GB_MBC5

#include "../cartridge.h"

typedef struct {
    uint8_t RAMG;                               // RAM gate register
    uint8_t BANK_ROM_H;                         // high single bit of the 9-bit ROM bank number
    uint8_t BANK_ROM_L;                         // lower 8 bits of the 9-bit ROM bank number
    uint8_t BANK_RAM;                           // 4 bit RAM bank number / bit 3 may be rumble control

    bool has_rumble;
    cart_data_t *cart;
} gb_mbc5_t;

void init_gameboy_mbc5(gb_cartridge_t *, bool use_rumble);

#endif