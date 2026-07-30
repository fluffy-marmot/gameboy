#include "../cartridge.h"

typedef struct {
    uint8_t RAMG;                               // RAM gate register
    uint8_t BANK1;                              // lower 5 bits of ROM bank number
    uint8_t BANK2;                              // upper 2 bits of ROM bank number or all RAM bank number
    uint8_t MODE;                               // mode register

    gb_cart_data_t *cart;
} gb_mbc1;

void init_gameboy_mbc1(gb_cartridge_t *);
/*

A15 A14 A13   Address range        Region
 0   0   0    $0000-$1FFF          RAM Enable                           register Ram "gate?"
 0   0   1    $2000-$3FFF          ROM Bank Number (low bits)
 0   1   0    $4000-$5FFF          RAM Bank / ROM Bank (high bits)
 0   1   1    $6000-$7FFF          Banking Mode Select
 1   0   1    $A000-$BFFF          External RAM access
 
 */