#include "mbc3.h"

#define USEPINS_RAMG                            0b00001111
#define USEPINS_BANK_ROM                        0b01111111

#define RAMG_WRITE_ACCESS_LOW                   0x0000
#define RAMG_WRITE_ACCESS_HIGH                  0x1FFF

#define BANK_ROM_WRITE_ACCESS_LOW               0x2000
#define BANK_ROM_WRITE_ACCESS_HIGH              0x3FFF

#define BANK_RAM_WRITE_ACCESS_LOW               0x4000
#define BANK_RAM_WRITE_ACCESS_HIGH              0x5FFF

#define RAMG_ENABLE_ACCESSS                     0b1010
#define BANK_ROM_INITIAL                        0b00000001


gb_mbc3_t mbc3;

static uint16_t
resolved_ram_index(memaddr address)
{
    uint16_t resolved = 0x2000 * (mbc3.BANK_RAM) + (address - ADDR_START_WRAM_CARTRIDGE);
    return resolved % mbc3.cart->ram_size;
}

static uint32_t
resolved_rom_index(memaddr address)
{
    uint32_t resolved = 0x4000 * (mbc3.BANK_ROM) + (address - ADDR_START_ROM_BANK);
    return resolved % mbc3.cart->rom_size;
}

static uint8_t
read_bus_mbc3(memaddr address)
{
    if (ADDR_BELOW(ADDR_END_ROM_FIXED)) {
        return mbc3.cart->rom[address];
    } else if (ADDR_RANGE(ADDR_START_ROM_BANK, ADDR_END_ROM_BANK)) {
        return mbc3.cart->rom[resolved_rom_index(address)];
    } else if (ADDR_RANGE(ADDR_START_WRAM_CARTRIDGE, ADDR_END_WRAM_CARTRIDGE)) {
        if (mbc3.RAMG != RAMG_ENABLE_ACCESSS)
            return UNREADABLE;

        if (mbc3.BANK_RAM <= 0b11 && mbc3.cart->ram_size > 0)
            return mbc3.cart->ram[resolved_ram_index(address)];
        else if (mbc3.rtc != NULL)
            return mbc3.rtc->bus_rtc->read(address);
    }
    return UNREADABLE;
}

static void
write_bus_mbc3(memaddr address, uint8_t val)
{
    if (ADDR_BELOW(RAMG_WRITE_ACCESS_HIGH)) {
        mbc3.RAMG = val & USEPINS_RAMG;
    } else if (ADDR_RANGE(BANK_ROM_WRITE_ACCESS_LOW, BANK_ROM_WRITE_ACCESS_HIGH)) {
        mbc3.BANK_ROM = ((val & USEPINS_BANK_ROM) == 0 ? BANK_ROM_INITIAL : val & USEPINS_BANK_ROM);
    } else if (ADDR_RANGE(BANK_RAM_WRITE_ACCESS_LOW, BANK_RAM_WRITE_ACCESS_HIGH)) {
        mbc3.BANK_RAM = val;
        if (mbc3.rtc != NULL)
            mbc3.rtc->bus_rtc->write(address, val);
    } else if (ADDR_RANGE(RTC_LATCH_ACCESS_LOW, RTC_LATCH_ACCESS_HIGH) && mbc3.rtc != NULL) {
        mbc3.rtc->bus_rtc->write(address, val);
    } else if (ADDR_RANGE(ADDR_START_WRAM_CARTRIDGE, ADDR_END_WRAM_CARTRIDGE)) {
        if (mbc3.RAMG != RAMG_ENABLE_ACCESSS)
            return;

        if (mbc3.BANK_RAM <= 0b11 && mbc3.cart->ram_size > 0)
            mbc3.cart->ram[resolved_ram_index(address)] = val;
        else if (mbc3.rtc != NULL)
            mbc3.rtc->bus_rtc->write(address, val);
    }
}
static bus_interface_t bus_mbc3 = { .read = read_bus_mbc3, .write = write_bus_mbc3 };

void
init_gameboy_mbc3(gb_cartridge_t *cartridge, bool use_rtc)
{
    memset(&mbc3, 0, sizeof(gb_mbc3_t));
    mbc3.BANK_ROM = BANK_ROM_INITIAL;
    mbc3.cart = &cartridge->data;
    if (use_rtc)
        mbc3.rtc = init_gameboy_rtc();
    cartridge->mbc_bus = &bus_mbc3;
}
