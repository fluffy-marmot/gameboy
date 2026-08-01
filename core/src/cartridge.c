#include "_abi.h"
#include "cartridge.h"
#include "mbcs/mbc1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static gb_cartridge_t cartridge;

static uint8_t 
read_cart (memaddr address)
{
    if (cartridge.mbc_bus)
        return cartridge.mbc_bus->read(address);
    
    if ADDR_BELOW(ADDR_END_ROM_BANK)
        return cartridge.data.rom[address];
    else if ((size_t) (address - ADDR_START_WRAM_CARTRIDGE) < cartridge.data.ram_size)
        // this is purely defensive, cartridges with no MBC aren't known to use external RAM
        return cartridge.data.ram[address - ADDR_START_WRAM_CARTRIDGE];
    else
        return UNREADABLE;
}

static void
write_cart(memaddr address, uint8_t val)
{
    if (cartridge.mbc_bus)
        cartridge.mbc_bus->write(address, val);
    else if ((size_t) (address - ADDR_START_WRAM_CARTRIDGE) < cartridge.data.ram_size)
        cartridge.data.ram[address - ADDR_START_WRAM_CARTRIDGE] = val;
}

static bus_interface_t bus_cart =  { .read = read_cart, .write = write_cart };

static size_t
reported_header_rom_size(void)
{
    switch (cartridge.header.rom_size_id) {
    case MBC_ROM_SIZE_32KiB:                    return   32 * KiB;
    case MBC_ROM_SIZE_64KiB:                    return   64 * KiB;
    case MBC_ROM_SIZE_128KiB:                   return  128 * KiB;
    case MBC_ROM_SIZE_256KiB:                   return  256 * KiB;
    case MBC_ROM_SIZE_512KiB:                   return  512 * KiB;
    case MBC_ROM_SIZE_1MiB:                     return    1 * MiB;
    case MBC_ROM_SIZE_2MiB:                     return    2 * MiB;
    case MBC_ROM_SIZE_4MiB:                     return    4 * MiB;
    case MBC_ROM_SIZE_8MiB:                     return    8 * MiB;
    case MBC_ROM_SIZE_1152KiB:                  return 1152 * KiB;
    case MBC_ROM_SIZE_1280KiB:                  return 1280 * KiB;
    case MBC_ROM_SIZE_1536KiB:                  return 1536 * KiB;
    default:                                    return    0      ;
    }
}

static gb_return_t
validate_header_ram_size(void)
{
    free(cartridge.data.ram);
    cartridge.data.ram = NULL;
    cartridge.data.ram_size = 0;

    switch (cartridge.header.ram_size_id) {
    case MBC_RAM_SIZE_NONE:                                                                     break;  
    case MBC_RAM_SIZE_2KiB:                     cartridge.data.ram_size =   2 * KiB;            break;
    case MBC_RAM_SIZE_8KiB:                     cartridge.data.ram_size =   8 * KiB;            break;
    case MBC_RAM_SIZE_32KiB:                    cartridge.data.ram_size =  32 * KiB;            break;
    case MBC_RAM_SIZE_128KiB:                   cartridge.data.ram_size = 128 * KiB;            break;
    case MBC_RAM_SIZE_64KiB:                    cartridge.data.ram_size =  64 * KiB;            break;
    default:
        return GB_ERROR_RAM_SIZE_HEADER_INVALID;
    }
    if (cartridge.data.ram_size > 0)
        cartridge.data.ram = calloc(cartridge.data.ram_size, sizeof(uint8_t));
    
    printf("%zu\n", cartridge.data.ram_size);
    return GB_RETURN_OK;
}

static gb_return_t
select_mbc_controller(void)
{
    switch (cartridge.header.mbc_type) {
    case MBC_TYPE_NONE_PLAIN_ROM:               cartridge.mbc_bus = NULL;                       break;
    case MBC_TYPE_MBC1:
    case MBC_TYPE_MBC1_RAM:
    case MBC_TYPE_MBC1_BBRAM:                   init_gameboy_mbc1(&cartridge);                  break;
    case MBC_TYPE_MBC2:
    case MBC_TYPE_MBC2_BBRAM:
    case MBC_TYPE_NONE_RAM:
    case MBC_TYPE_NONE_BBRAM:
    case MBC_TYPE_MMM01:
    case MBC_TYPE_MMM01_RAM:
    case MBC_TYPE_MMM01_BBRAM:
    case MBC_TYPE_MBC3_RTCLOCK:
    case MBC_TYPE_MBC3_RTCLOCK_BBRAM:
    case MBC_TYPE_MBC3:
    case MBC_TYPE_MBC3_RAM:
    case MBC_TYPE_MBC3_BBRAM:
    case MBC_TYPE_MBC5:
    case MBC_TYPE_MBC5_RAM:
    case MBC_TYPE_MBC5_BBRAM:
    case MBC_TYPE_MBC5_RUMBLE:
    case MBC_TYPE_MBC5_RUMBLE_RAM:
    case MBC_TYPE_MBC5_RUMBLE_BBRAM:
    case MBC_TYPE_MBC6:
    case MBC_TYPE_MBC7:
    case MBC_TYPE_POCKET_CAMERA:
    case MBC_TYPE_BANDAI_TAMAS:
    case MBC_TYPE_HuC3:
    case MBC_TYPE_HuC1:                         return GB_ERROR_MBC_UNIMPLEMENTED;
    default:                                    return GB_ERROR_MBC_UNRECOGNIZED;
    }
    return GB_RETURN_OK;
}

gb_cartridge_t *
init_cartridge(gb_bus_t *bus)
{
    bus->interface_cartridge = &bus_cart;
    return &cartridge;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions
        
###############################################################################
############################################################################ */

gb_return_t
GB_load_rom(const uint8_t *data, size_t size)
{
    if (size < 32 * KiB)
        return GB_ERROR_ROM_SIZE_MINIMUM;
    free(cartridge.data.rom);
    cartridge.data.rom = (uint8_t *) malloc(size * sizeof(uint8_t));
    if (!cartridge.data.rom)
        return GB_ERROR_MALLOC;
    memcpy(cartridge.data.rom, data, size);

    // fill in header info
    cartridge.header.mbc_type = cartridge.data.rom[CARTRIDGE_HEADER_MBC_TYPE];
    cartridge.header.rom_size_id = cartridge.data.rom[CARTRIDGE_HEADER_ROM_SIZE];
    cartridge.header.ram_size_id = cartridge.data.rom[CARTRIDGE_HEADER_RAM_SIZE];

    printf("ROM header: %X %X %X\n", cartridge.header.mbc_type, cartridge.header.rom_size_id, cartridge.header.ram_size_id);

    size_t rom_size = reported_header_rom_size();
    if (rom_size == 0)
        return GB_ERROR_ROM_SIZE_HEADER_INVALID;
    if (reported_header_rom_size() != size)
        return GB_ERROR_ROM_SIZE_HEADER_MISMATCH;
    cartridge.data.rom_size = size;

    gb_return_t mbc_validate_ram_result = validate_header_ram_size();
    if (mbc_validate_ram_result != GB_RETURN_OK)
        return mbc_validate_ram_result;

    gb_return_t mbc_select_result = select_mbc_controller();
    if (mbc_select_result != GB_RETURN_OK)
        return mbc_select_result;

    return GB_RETURN_OK;
}