#include "mbc5.h"

#define USEPINS_RAMG                            0b00001111
#define USEPINS_BANK_ROM_H                      0b00000001
#define USEPINS_BANK_RAM                        0b00001111

#define RAMG_WRITE_ACCESS_LOW                   0x0000
#define RAMG_WRITE_ACCESS_HIGH                  0x1FFF

#define BANK_ROM_L_WRITE_ACCESS_LOW             0x2000
#define BANK_ROM_L_WRITE_ACCESS_HIGH            0x2FFF

#define BANK_ROM_H_WRITE_ACCESS_LOW             0x3000
#define BANK_ROM_H_WRITE_ACCESS_HIGH            0x3FFF

#define BANK_RAM_WRITE_ACCESS_LOW               0x4000
#define BANK_RAM_WRITE_ACCESS_HIGH              0x5FFF

#define RAMG_ENABLE_ACCESSS                     0b1010

#define ROM_BANK                                ((mbc5.BANK_ROM_H << 8) | mbc5.BANK_ROM_L)
#define RAM_BANK                                (mbc5.BANK_RAM & (mbc5.has_rumble ? 0b0111 : 0b1111))

gb_mbc5_t mbc5;

static uint32_t
resolved_ram_index(memaddr address)
{
    uint32_t resolved = 0x2000 * RAM_BANK + (address - ADDR_START_WRAM_CARTRIDGE);
    return resolved % mbc5.cart->ram_size;
}

static uint32_t
resolved_rom_index(memaddr address)
{
    uint32_t resolved = 0x4000 * ROM_BANK + (address - ADDR_START_ROM_BANK);
    return resolved % mbc5.cart->rom_size;
}

static uint8_t
read_bus_mbc5(memaddr address)
{
    if (ADDR_BELOW(ADDR_END_ROM_FIXED)) {
        return mbc5.cart->rom[address];
    } else if (ADDR_RANGE(ADDR_START_ROM_BANK, ADDR_END_ROM_BANK)) {
        return mbc5.cart->rom[resolved_rom_index(address)];
    } else if (ADDR_RANGE(ADDR_START_WRAM_CARTRIDGE, ADDR_END_WRAM_CARTRIDGE)) {
        if (mbc5.RAMG == RAMG_ENABLE_ACCESSS && mbc5.cart->ram_size > 0)
            return mbc5.cart->ram[resolved_ram_index(address)];
    }
    return UNREADABLE;
}

static void
write_bus_mbc5(memaddr address, uint8_t val)
{
    if (ADDR_BELOW(RAMG_WRITE_ACCESS_HIGH)) {
        mbc5.RAMG = val & USEPINS_RAMG;
    } else if (ADDR_RANGE(BANK_ROM_L_WRITE_ACCESS_LOW, BANK_ROM_L_WRITE_ACCESS_HIGH)) {
        mbc5.BANK_ROM_L = val;
    } else if (ADDR_RANGE(BANK_ROM_H_WRITE_ACCESS_LOW, BANK_ROM_H_WRITE_ACCESS_HIGH)) {
        mbc5.BANK_ROM_H = val & USEPINS_BANK_ROM_H;
    } else if (ADDR_RANGE(BANK_RAM_WRITE_ACCESS_LOW, BANK_RAM_WRITE_ACCESS_HIGH)) {
        mbc5.BANK_RAM = val & USEPINS_BANK_RAM;
    } else if (ADDR_RANGE(ADDR_START_WRAM_CARTRIDGE, ADDR_END_WRAM_CARTRIDGE)) {
        if (mbc5.RAMG == RAMG_ENABLE_ACCESSS && mbc5.cart->ram_size > 0)
            mbc5.cart->ram[resolved_ram_index(address)] = val;
    }
}
static bus_interface_t bus_mbc5 = { .read = read_bus_mbc5, .write = write_bus_mbc5 };

void
init_gameboy_mbc5(gb_cartridge_t *cartridge, bool use_rumble)
{
    memset(&mbc5, 0, sizeof(gb_mbc5_t));
    mbc5.BANK_ROM_L = 0x01;
    mbc5.has_rumble = use_rumble;
    mbc5.cart = &cartridge->data;
    cartridge->mbc_bus = &bus_mbc5;
}
