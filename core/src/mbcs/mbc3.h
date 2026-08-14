#ifndef GB_MBC3
#define GB_MBC3

#include "../cartridge.h"
#include "rtc.h"

typedef struct {
    uint8_t RAMG;                               // RAM gate register
    uint8_t BANK_ROM;                           // 7 bit ROM bank number
    uint8_t BANK_RAM;                           // 2 bit RAM bank number / RTC register select

    gb_rtc_t *rtc;

    cart_data_t *cart;
} gb_mbc3_t;

void init_gameboy_mbc3(gb_cartridge_t *, bool use_rtc);

#endif