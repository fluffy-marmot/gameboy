#include "mbc1.h"

#define USEPINS_RAMG                            0b00001111
#define USEPINS_BANK1                           0b00011111
#define USEPINS_BANK2                           0b00000011
#define USEPINS_MODE                            0b00000001

#define RAMG_WRITE_ACCESS_LOW                   0x0000
#define RAMG_WRITE_ACCESS_HIGH                  0x1FFF
#define RAMG_ENABLE_ACCESSS                     0b1010

#define BANK1_WRITE_ACCESS_LOW                  0x2000
#define BANK1_WRITE_ACCESS_HIGH                 0x3FFF
#define BANK1_INITIAL                           0b00001

#define BANK2_WRITE_ACCESS_LOW                  0x4000
#define BANK2_WRITE_ACCESS_HIGH                 0x5FFF

#define MODE_WRITE_ACCESS_LOW                   0x6000
#define MODE_WRITE_ACCESS_HIGH                  0x7FFF

gb_mbc1_t mbc1;

static uint8_t
high_bank_number(void)
{
    switch (mbc1.cart->rom_size) {
    case 2 * MiB:
        return ((mbc1.BANK2 & 0b11) << 5) | (mbc1.BANK1 & mbc1.rom_bank_bitmask);
    case 1 * MiB:
        return ((mbc1.BANK2 & 0b01) << 5) | (mbc1.BANK1 & mbc1.rom_bank_bitmask);
    default:
        return (mbc1.BANK1 & mbc1.rom_bank_bitmask);
    }
}

static uint8_t
zero_bank_number(void)
{
    switch (mbc1.cart->rom_size) {
    case 2 * MiB:
        return ((mbc1.BANK2 & 0b11) << 5);
    case 1 * MiB:
        return ((mbc1.BANK2 & 0b01) << 5);
    default:
        return 0;
    }
}

static uint16_t
resolved_ram_index(memaddr address)
{
    if (mbc1.cart->ram_size == 2 * KiB || mbc1.cart->ram_size == 8 * KiB)
        return (address - ADDR_START_WRAM_CARTRIDGE) % mbc1.cart->ram_size;
    else if (mbc1.MODE == 0)
        return address - ADDR_START_WRAM_CARTRIDGE;
    else
        return (mbc1.BANK2 << 13) | (address - ADDR_START_WRAM_CARTRIDGE);
}

static uint8_t
read_bus_mbc1(memaddr address)
{
    if (ADDR_BELOW(ADDR_END_ROM_FIXED)) {
        if (mbc1.MODE == 0)
            return mbc1.cart->rom[address];
        else
            return mbc1.cart->rom[zero_bank_number() * ADDR_START_ROM_BANK + address];
    }
    if (ADDR_RANGE(ADDR_START_ROM_BANK, ADDR_END_ROM_BANK)) {
        return mbc1.cart->rom[high_bank_number() * ADDR_START_ROM_BANK + (address - ADDR_START_ROM_BANK)];
    }
    if (ADDR_RANGE(ADDR_START_WRAM_CARTRIDGE, ADDR_END_WRAM_CARTRIDGE)) {
        if (mbc1.RAMG != RAMG_ENABLE_ACCESSS || mbc1.cart->ram_size == 0)
            return UNREADABLE;

        uint16_t ram_index = resolved_ram_index(address);

        if (ram_index < mbc1.cart->ram_size)
            return mbc1.cart->ram[ram_index];
    }
    return UNREADABLE;
}

static void
write_bus_mbc1(memaddr address, uint8_t val)
{
    if (ADDR_BELOW(RAMG_WRITE_ACCESS_HIGH)) {
        mbc1.RAMG = val & USEPINS_RAMG;
    } else if (ADDR_RANGE(BANK1_WRITE_ACCESS_LOW, BANK1_WRITE_ACCESS_HIGH)) {
        mbc1.BANK1 = ((val & USEPINS_BANK1) == 0 ? BANK1_INITIAL : val & USEPINS_BANK1);
    } else if (ADDR_RANGE(BANK2_WRITE_ACCESS_LOW, BANK2_WRITE_ACCESS_HIGH)) {
        mbc1.BANK2 = val & USEPINS_BANK2;
    } else if (ADDR_RANGE(MODE_WRITE_ACCESS_LOW, MODE_WRITE_ACCESS_HIGH)) {
        mbc1.MODE = val & USEPINS_MODE;
    }

    else if (ADDR_RANGE(ADDR_START_WRAM_CARTRIDGE, ADDR_END_WRAM_CARTRIDGE)) {
        if (mbc1.RAMG != RAMG_ENABLE_ACCESSS || mbc1.cart->ram_size == 0)
            return;

        uint16_t ram_index = resolved_ram_index(address);

        if (ram_index < mbc1.cart->ram_size)
            mbc1.cart->ram[ram_index] = val;
    }
}
static bus_interface_t bus_mbc1 = { .read = read_bus_mbc1, .write = write_bus_mbc1 };

static uint8_t
rom_bank_bitmask(header_rom_size_t rom_size_id)
{
    switch (rom_size_id) {
    case MBC_ROM_SIZE_32KiB:                    return 0b00000001;
    case MBC_ROM_SIZE_64KiB:                    return 0b00000011;
    case MBC_ROM_SIZE_128KiB:                   return 0b00000111;
    case MBC_ROM_SIZE_256KiB:                   return 0b00001111;
    case MBC_ROM_SIZE_512KiB:
    case MBC_ROM_SIZE_1MiB:
    case MBC_ROM_SIZE_2MiB:
    case MBC_ROM_SIZE_4MiB:
    case MBC_ROM_SIZE_8MiB:
    case MBC_ROM_SIZE_1152KiB:
    case MBC_ROM_SIZE_1280KiB:
    case MBC_ROM_SIZE_1536KiB:
    default:                                    return 0b00011111;
    }
}

void
init_gameboy_mbc1(gb_cartridge_t *cartridge, header_rom_size_t rom_size_id)
{
    memset(&mbc1, 0, sizeof(gb_mbc1_t));
    mbc1.BANK1 = BANK1_INITIAL;
    mbc1.rom_bank_bitmask = rom_bank_bitmask(rom_size_id);
    mbc1.cart = &cartridge->data;
    cartridge->mbc_bus = &bus_mbc1;
}
