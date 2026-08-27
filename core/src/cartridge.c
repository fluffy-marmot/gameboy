#include "_abi.h"
#include "cartridge.h"
#include "mbcs/mbc1.h"
#include "mbcs/mbc3.h"
#include "mbcs/mbc5.h"

const uint8_t NINTENDO_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

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

static gb_return_t
select_mbc_controller(void)
{
    header_rom_size_t rom_size_id = cartridge.header.rom_size_id;
    bool use_rtc = false;
    bool use_rumble = false;

    switch (cartridge.header.mbc_type) {
    case MBC_TYPE_NONE_PLAIN_ROM:
    case MBC_TYPE_NONE_RAM:
    case MBC_TYPE_NONE_BBRAM:                   cartridge.mbc_bus = NULL;                       break;

    case MBC_TYPE_MBC1:
    case MBC_TYPE_MBC1_RAM:
    case MBC_TYPE_MBC1_BBRAM:                   init_gameboy_mbc1(&cartridge, rom_size_id);     break;

    case MBC_TYPE_MBC3_RTCLOCK:
    case MBC_TYPE_MBC3_RTCLOCK_BBRAM:           use_rtc = true;                     /* fall through */
    case MBC_TYPE_MBC3:
    case MBC_TYPE_MBC3_RAM:
    case MBC_TYPE_MBC3_BBRAM:                   init_gameboy_mbc3(&cartridge, use_rtc);         break;

    case MBC_TYPE_MBC5_RUMBLE:
    case MBC_TYPE_MBC5_RUMBLE_RAM:
    case MBC_TYPE_MBC5_RUMBLE_BBRAM:            use_rumble = true;                  /* fall through */
    case MBC_TYPE_MBC5:
    case MBC_TYPE_MBC5_RAM:
    case MBC_TYPE_MBC5_BBRAM:                   init_gameboy_mbc5(&cartridge, use_rumble);      break;

    case MBC_TYPE_MBC2:
    case MBC_TYPE_MBC2_BBRAM:
    case MBC_TYPE_MMM01:
    case MBC_TYPE_MMM01_RAM:
    case MBC_TYPE_MMM01_BBRAM:

    case MBC_TYPE_MBC6:
    case MBC_TYPE_MBC7:
    case MBC_TYPE_POCKET_CAMERA:
    case MBC_TYPE_BANDAI_TAMA5:
    case MBC_TYPE_HuC3:
    case MBC_TYPE_HuC1:                         return GB_ERROR_MBC_UNIMPLEMENTED;
    default:                                    return GB_ERROR_MBC_UNRECOGNIZED;
    }
    return GB_RETURN_OK;
}

static gb_return_t
copy_rom_data(const uint8_t *data, size_t size)
{
    free(cartridge.data.rom);
    cartridge.data.rom = NULL;
    cartridge.data.rom_size = 0;

    if (size < 32 * KiB)
        return GB_ERROR_ROM_SIZE_MINIMUM;
    cartridge.data.rom = (uint8_t *) malloc(size * sizeof(uint8_t));
    if (!cartridge.data.rom)
        return GB_ERROR_MALLOC;
    memcpy(cartridge.data.rom, data, size);
    cartridge.data.rom_size = size;

    return GB_RETURN_OK;
}

/* ############################################################################
###############################################################################

        Various functions to parse and validate the cartridge header info

###############################################################################
############################################################################ */


static gb_return_t
validate_header_rom_size(size_t romdata_size)
{
    switch (cartridge.header.rom_size_id) {
    case MBC_ROM_SIZE_32KiB:                    cartridge.data.rom_size =   32 * KiB;           break;
    case MBC_ROM_SIZE_64KiB:                    cartridge.data.rom_size =   64 * KiB;           break;
    case MBC_ROM_SIZE_128KiB:                   cartridge.data.rom_size =  128 * KiB;           break;
    case MBC_ROM_SIZE_256KiB:                   cartridge.data.rom_size =  256 * KiB;           break;
    case MBC_ROM_SIZE_512KiB:                   cartridge.data.rom_size =  512 * KiB;           break;
    case MBC_ROM_SIZE_1MiB:                     cartridge.data.rom_size =    1 * MiB;           break;
    case MBC_ROM_SIZE_2MiB:                     cartridge.data.rom_size =    2 * MiB;           break;
    case MBC_ROM_SIZE_4MiB:                     cartridge.data.rom_size =    4 * MiB;           break;
    case MBC_ROM_SIZE_8MiB:                     cartridge.data.rom_size =    8 * MiB;           break;
    case MBC_ROM_SIZE_1152KiB:                  cartridge.data.rom_size = 1152 * KiB;           break;
    case MBC_ROM_SIZE_1280KiB:                  cartridge.data.rom_size = 1280 * KiB;           break;
    case MBC_ROM_SIZE_1536KiB:                  cartridge.data.rom_size = 1536 * KiB;           break;
    default:
        return GB_ERROR_ROM_SIZE_HEADER_INVALID;
    }
    if (cartridge.data.rom_size != romdata_size)
        return GB_ERROR_ROM_SIZE_HEADER_MISMATCH;

    return GB_RETURN_OK;
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

    return GB_RETURN_OK;
}

static void
read_cartridge_header_check_nintendo_logo(void)
{
    for (uint8_t byte = 0; byte < sizeof(NINTENDO_LOGO); byte++)
        if (NINTENDO_LOGO[byte] != cartridge.data.rom[CARTRIDGE_HEADER_NINTENDO_LOGO + byte]) {
            cartridge.header.readable.logo_ok = "Invalid";
            return;
        }
    cartridge.header.readable.logo_ok = "Ok";
}

static gb_return_t
read_cartridge_header_title_and_manufacturer(void)
{
    #define PRINTABLE_ASCII(byte) (0x20 <= (byte) && (byte) < 0x80)

    free(cartridge.header.readable.title);
    free(cartridge.header.readable.manufacturer);
    cartridge.header.readable.title = NULL;
    cartridge.header.readable.manufacturer = NULL;

    // determine the title string
    uint8_t max_title_len = 16, title_len;
    switch (cartridge.header.cgb_flag) {
    case CGB_FLAG_CGB_ONLY:
    case CGB_FLAG_BACKWARDS_COMPATIBLE:
        max_title_len = 15;
    }
    for (title_len = 0; title_len < max_title_len; title_len++) {
        if (!PRINTABLE_ASCII(cartridge.data.rom[CARTRIDGE_HEADER_TITLE + title_len]))
            break;
    }
    cartridge.header.readable.title = (char *) calloc(title_len + 1, sizeof(char));
    if (!cartridge.header.readable.title)
        return GB_ERROR_MALLOC;
    memcpy(cartridge.header.readable.title, &cartridge.data.rom[CARTRIDGE_HEADER_TITLE], title_len);

    // if the cartridge is not DMG, possibly determine a manufacturer string, otherwise leave blank
    cartridge.header.readable.manufacturer = (char *) calloc(5, sizeof(char));
    if (!cartridge.header.readable.manufacturer)
        return GB_ERROR_MALLOC;
    if (max_title_len < 16 && title_len <= 11)
        memcpy(cartridge.header.readable.manufacturer, &cartridge.data.rom[CARTRIDGE_HEADER_MANUFACTURER], 4);

    #undef PRINTABLE_ASCII
    return GB_RETURN_OK;
}

static void
read_cartridge_header_mbc_type(void)
{
    char *mbc_type = "";
    switch (cartridge.header.mbc_type) {
    case MBC_TYPE_NONE_PLAIN_ROM:               mbc_type = "No MBC";                            break;
    case MBC_TYPE_MBC1:                         mbc_type = "MBC1";                              break;
    case MBC_TYPE_MBC1_RAM:                     mbc_type = "MBC1, RAM";                         break;
    case MBC_TYPE_MBC1_BBRAM:                   mbc_type = "MBC1, BBRAM";                       break;
    case MBC_TYPE_MBC2:                         mbc_type = "MBC2";                              break;
    case MBC_TYPE_MBC2_BBRAM:                   mbc_type = "MBC2, BBRAM";                       break;
    case MBC_TYPE_NONE_RAM:                     mbc_type = "No MBC, RAM";                       break;
    case MBC_TYPE_NONE_BBRAM:                   mbc_type = "No MBC, BBRAM";                     break;
    case MBC_TYPE_MMM01:                        mbc_type = "MMM01";                             break;
    case MBC_TYPE_MMM01_RAM:                    mbc_type = "MMM01, RAM";                        break;
    case MBC_TYPE_MMM01_BBRAM:                  mbc_type = "MMM01, BBRAM";                      break;
    case MBC_TYPE_MBC3_RTCLOCK:                 mbc_type = "MBC3, RTC";                         break;
    case MBC_TYPE_MBC3_RTCLOCK_BBRAM:           mbc_type = "MBC3, BBRAM, RTC";                  break;
    case MBC_TYPE_MBC3:                         mbc_type = "MBC3";                              break;
    case MBC_TYPE_MBC3_RAM:                     mbc_type = "MBC3, RAM";                         break;
    case MBC_TYPE_MBC3_BBRAM:                   mbc_type = "MBC3, BBRAM";                       break;
    case MBC_TYPE_MBC5:                         mbc_type = "MBC5";                              break;
    case MBC_TYPE_MBC5_RAM:                     mbc_type = "MBC5, RAM";                         break;
    case MBC_TYPE_MBC5_BBRAM:                   mbc_type = "MBC5, BBRAM";                       break;
    case MBC_TYPE_MBC5_RUMBLE:                  mbc_type = "MBC5, Rumble";                      break;
    case MBC_TYPE_MBC5_RUMBLE_RAM:              mbc_type = "MBC5, RAM, Rumble";                 break;
    case MBC_TYPE_MBC5_RUMBLE_BBRAM:            mbc_type = "MBC5, BBRAM, Rumble";               break;
    case MBC_TYPE_MBC6:                         mbc_type = "MBC6";                              break;
    case MBC_TYPE_MBC7:                         mbc_type = "MBC7";                              break;
    case MBC_TYPE_POCKET_CAMERA:                mbc_type = "Pocket Camera";                     break;
    case MBC_TYPE_BANDAI_TAMA5:                 mbc_type = "Bandai Tama5";                      break;
    case MBC_TYPE_HuC3:                         mbc_type = "HuC3";                              break;
    case MBC_TYPE_HuC1:                         mbc_type = "HuC1";                              break;
    default:                                    mbc_type = "Unknown";                           break;
    }

    cartridge.header.readable.mbc_type = mbc_type;
}

static void
read_cartridge_header_rom_size(void)
{
    switch (cartridge.header.rom_size_id) {
    case MBC_ROM_SIZE_32KiB:                    cartridge.header.readable.rom_size = "32 KiB";  break;
    case MBC_ROM_SIZE_64KiB:                    cartridge.header.readable.rom_size = "64 KiB";  break;
    case MBC_ROM_SIZE_128KiB:                   cartridge.header.readable.rom_size = "128 KiB"; break;
    case MBC_ROM_SIZE_256KiB:                   cartridge.header.readable.rom_size = "256 KiB"; break;
    case MBC_ROM_SIZE_512KiB:                   cartridge.header.readable.rom_size = "512 KiB"; break;
    case MBC_ROM_SIZE_1MiB:                     cartridge.header.readable.rom_size = "1 MiB";   break;
    case MBC_ROM_SIZE_2MiB:                     cartridge.header.readable.rom_size = "2 MiB";   break;
    case MBC_ROM_SIZE_4MiB:                     cartridge.header.readable.rom_size = "4 MiB";   break;
    case MBC_ROM_SIZE_8MiB:                     cartridge.header.readable.rom_size = "8 MiB";   break;
    case MBC_ROM_SIZE_1152KiB:                  cartridge.header.readable.rom_size = "1.1 MiB"; break;
    case MBC_ROM_SIZE_1280KiB:                  cartridge.header.readable.rom_size = "1.2 MiB"; break;
    case MBC_ROM_SIZE_1536KiB:                  cartridge.header.readable.rom_size = "1.3 MiB"; break;
    default:                                    cartridge.header.readable.rom_size = "Invalid"; break;
    }
}

static void
read_cartridge_header_ram_size(void)
{
    switch (cartridge.header.ram_size_id) {
    case MBC_RAM_SIZE_NONE:                     cartridge.header.readable.ram_size = "-";    break;
    case MBC_RAM_SIZE_2KiB:                     cartridge.header.readable.ram_size = "2 KiB";   break;
    case MBC_RAM_SIZE_8KiB:                     cartridge.header.readable.ram_size = "8 KiB";   break;
    case MBC_RAM_SIZE_32KiB:                    cartridge.header.readable.ram_size = "32 KiB";  break;
    case MBC_RAM_SIZE_128KiB:                   cartridge.header.readable.ram_size = "128 KiB"; break;
    case MBC_RAM_SIZE_64KiB:                    cartridge.header.readable.ram_size = "64 KiB";  break;
    default:                                    cartridge.header.readable.ram_size = "Invalid"; break;
    }
}

static void
read_cartridge_header_perform_header_checksum(void)
{
    uint8_t expected = cartridge.data.rom[CARTRIDGE_HEADER_CHECKSUM];
    uint8_t checksum = 0;
    for (memaddr addr = ADDR_START_HEADER_CHECKSUM_RANGE; addr <= ADDR_END_HEADER_CHECKSUM_RANGE; addr++)
        checksum = checksum - cartridge.data.rom[addr] - 1;

    cartridge.header.readable.checksum_ok = checksum == expected ? "Ok" : "Failed";
}

static void
read_cartridge_header_perform_global_checksum(void)
{
    uint16_t expected = (cartridge.data.rom[CARTRIDGE_HEADER_GLOBAL_CHECKSUM] << 8) +
                        (cartridge.data.rom[CARTRIDGE_HEADER_GLOBAL_CHECKSUM + 1] );
    uint16_t checksum = 0;
    for (size_t byte = 0; byte < cartridge.data.rom_size; byte++)
        checksum += cartridge.data.rom[byte];

    checksum -= cartridge.data.rom[CARTRIDGE_HEADER_GLOBAL_CHECKSUM];
    checksum -= cartridge.data.rom[CARTRIDGE_HEADER_GLOBAL_CHECKSUM + 1];

    cartridge.header.readable.global_checksum_ok = checksum == expected ? "Ok" : "Failed";
}

// call once with NULL, 0 to precompute length, 2nd time to write into actual buffer
static int
build_cartridge_header_json_helper(char *buf, size_t maxlen)
{
    // defensive check against null pointers in header.readable
    #define S(s)                                ((s) ? (s) : "")
    // allow this function to be called with *buf being NULL to only precompute the length of the json
    // in this case call snprintf with first two arguments NULL and 0
    #define BUF_PTR                             (buf ? buf + offset : NULL)
    #define BUF_POS                             (buf ? maxlen - offset : 0)
    #define JSON(fmt_string, fmt_arg)           (offset += snprintf(BUF_PTR, BUF_POS, (fmt_string), (fmt_arg)))

    int offset = 0;
    JSON("{\"logo_ok\": \"%s\",",               S(cartridge.header.readable.logo_ok));
    JSON("\"title\": \"%s\",",                  S(cartridge.header.readable.title));
    JSON("\"manufacturer\": \"%s\",",           S(cartridge.header.readable.manufacturer));
    JSON("\"cgb_flag\": \"%s\",",               S(cartridge.header.readable.cgb_flag));
    JSON("\"licensee\": \"%s\",",               S(cartridge.header.readable.licensee));
    JSON("\"sgb_flag\": \"%s\",",               S(cartridge.header.readable.sgb_flag));
    JSON("\"mbc_type\": \"%s\",",               S(cartridge.header.readable.mbc_type));
    JSON("\"rom_size\": \"%s\",",               S(cartridge.header.readable.rom_size));
    JSON("\"ram_size\": \"%s\",",               S(cartridge.header.readable.ram_size));
    JSON("\"destination\": \"%s\",",            S(cartridge.header.readable.destination));
    JSON("\"checksum_ok\": \"%s\",",            S(cartridge.header.readable.checksum_ok));
    JSON("\"global_checksum_ok\": \"%s\",",     S(cartridge.header.readable.global_checksum_ok));
    JSON("\"rom_version\": \"0x%02X\",",          cartridge.header.rom_version);
    JSON("\"mbc_type_id\": \"0x%02X\",",          cartridge.header.mbc_type);
    JSON("\"rom_size_id\": \"0x%02X\",",          cartridge.header.rom_size_id);
    JSON("\"ram_size_id\": \"0x%02X\",",          cartridge.header.ram_size_id);
    JSON("\"bbram_numbytes\": %zu}",            GB_uses_bbram() ? cartridge.data.ram_size : 0);

    #undef S
    #undef BUF_PTR
    #undef BUF_POS
    #undef JSON
    return offset;
}

static gb_return_t
build_cartridge_header_json(void)
{
    free(cartridge.header.json);
    cartridge.header.json = NULL;
    int json_len = build_cartridge_header_json_helper(NULL, 0) + 1;
    if (json_len < 0)
        return GB_ERROR_JSON_FORMAT;
    cartridge.header.json = (char *) malloc(json_len);
    if (!cartridge.header.json)
        return GB_ERROR_MALLOC;
    build_cartridge_header_json_helper(cartridge.header.json, json_len);
    return GB_RETURN_OK;
}

static gb_return_t
read_cartridge_header(void)
{
    gb_return_t return_status;

    cartridge.header.mbc_type                 = cartridge.data.rom[CARTRIDGE_HEADER_MBC_TYPE];
    cartridge.header.rom_size_id              = cartridge.data.rom[CARTRIDGE_HEADER_ROM_SIZE];
    cartridge.header.ram_size_id              = cartridge.data.rom[CARTRIDGE_HEADER_RAM_SIZE];
    cartridge.header.cgb_flag                 = cartridge.data.rom[CARTRIDGE_HEADER_CGB_FLAG];
    cartridge.header.sgb_flag                 = cartridge.data.rom[CARTRIDGE_HEADER_SGB_FLAG];
    cartridge.header.rom_version              = cartridge.data.rom[CARTRIDGE_HEADER_ROM_VERSION];

    read_cartridge_header_mbc_type();
    read_cartridge_header_rom_size();
    read_cartridge_header_ram_size();

    read_cartridge_header_check_nintendo_logo();

    switch (cartridge.header.cgb_flag) {
    case CGB_FLAG_CGB_ONLY:                     cartridge.header.readable.cgb_flag = "Exclusive";  break;
    case CGB_FLAG_BACKWARDS_COMPATIBLE:         cartridge.header.readable.cgb_flag = "Compatible"; break;
    default:                                    cartridge.header.readable.cgb_flag = "No";         break;
    }

    switch (cartridge.header.sgb_flag) {
    case SGB_FLAG_SGB_FUNCTIONS_SUPPORT:        cartridge.header.readable.sgb_flag = "Supported";  break;
    default:                                    cartridge.header.readable.sgb_flag = "No";         break;
    }

    switch (cartridge.header.destination_code) {
    case HEADER_DESTINATION_CODE_JAPAN:         cartridge.header.readable.destination = "Japan";         break;
    case HEADER_DESTINATION_CODE_OVERSEAS_ONLY: cartridge.header.readable.destination = "International"; break;
    default:                                    cartridge.header.readable.destination = "Unknown";       break;
    }

    // TODO - licensee old and new?

    read_cartridge_header_perform_header_checksum();
    read_cartridge_header_perform_global_checksum();

    return_status = read_cartridge_header_title_and_manufacturer();
    if (return_status != GB_RETURN_OK)
        return return_status;

    return GB_RETURN_OK;
}

gb_cartridge_t *
init_cartridge(gb_bus_t *bus)
{
    free(cartridge.header.readable.title);
    free(cartridge.header.readable.manufacturer);
    free(cartridge.header.json);
    free(cartridge.data.rom);
    free(cartridge.data.ram);
    memset(&cartridge, 0, sizeof(gb_cartridge_t));
    bus->interface_cartridge = &bus_cart;
    return &cartridge;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions

###############################################################################
############################################################################ */

uint8_t*
GB_get_cartridge_ram(void)
{
    return cartridge.data.ram;
}

bool
GB_uses_rumble(void)
{
    switch (cartridge.header.mbc_type) {
    case MBC_TYPE_MBC5_RUMBLE:
    case MBC_TYPE_MBC5_RUMBLE_RAM:
    case MBC_TYPE_MBC5_RUMBLE_BBRAM:
        return true;
    default:
        return false;
    }
}

bool
GB_uses_rtc(void)
{
    switch (cartridge.header.mbc_type) {
    case MBC_TYPE_MBC3_RTCLOCK_BBRAM:
    case MBC_TYPE_MBC3_RTCLOCK:
    case MBC_TYPE_HuC3:
        return true;
    default:
        return false;
    }
}

bool
GB_uses_bbram(void)
{
    switch (cartridge.header.mbc_type) {
    case MBC_TYPE_MBC1_BBRAM:
    case MBC_TYPE_MBC2_BBRAM:
    case MBC_TYPE_NONE_BBRAM:
    case MBC_TYPE_MMM01_BBRAM:
    case MBC_TYPE_MBC3_RTCLOCK_BBRAM:
    case MBC_TYPE_MBC3_BBRAM:
    case MBC_TYPE_MBC5_BBRAM:
    case MBC_TYPE_MBC5_RUMBLE_BBRAM:
    case MBC_TYPE_MBC7:
    case MBC_TYPE_HuC1:
        return true;
    default:
        return false;
    }
}

char*
GB_cartridge_header_as_json(void)
{
    return cartridge.header.json;
}

gb_return_t
GB_load_bbram(const uint8_t *data, size_t size)
{
    if (size != cartridge.data.ram_size)
        return GB_ERROR_BBRAM_WRONG_SIZE;
    memcpy(cartridge.data.ram, data, size);
    return GB_RETURN_OK;
}

gb_return_t
GB_load_rom(const uint8_t *data, size_t size)
{
    gb_return_t return_status;
    #define TRY(expr)                                                   \
        do {                                                            \
            return_status = (expr);                                     \
            if (return_status != GB_RETURN_OK) return return_status;    \
        } while (0)

    TRY(copy_rom_data(data, size));
    TRY(read_cartridge_header());
    TRY(validate_header_rom_size(size));
    TRY(validate_header_ram_size());
    TRY(select_mbc_controller());
    TRY(build_cartridge_header_json());

    #undef TRY
    return GB_RETURN_OK;
}
