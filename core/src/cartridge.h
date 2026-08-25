#ifndef GB_CARTRIDGE
#define GB_CARTRIDGE

#include "_specification.h"
#include "bus.h"
#include "mbcs/rtc.h"

typedef struct {
    uint8_t *rom;
    uint8_t *ram;
    size_t rom_size;
    size_t ram_size;
} cart_data_t;

typedef struct {
    struct {
        header_mbc_type_t mbc_type;
        header_rom_size_t rom_size_id;
        header_ram_size_t ram_size_id;
        header_cgb_flag_t cgb_flag;
        header_sgb_flag_t sgb_flag;
        header_des_code_t destination_code;
        uint8_t           rom_version;
        struct {
            char *logo_ok;
            char *title;
            char *manufacturer;
            char *cgb_flag;
            char *licensee;
            char *sgb_flag;
            char *mbc_type;
            char *rom_size;
            char *ram_size;
            char *destination;
            char *checksum_ok;
            char *global_checksum_ok;
        } readable;
        char *json;
    } header;

    cart_data_t data;
    bus_interface_t *mbc_bus;
} gb_cartridge_t;

gb_cartridge_t *init_cartridge(gb_bus_t *);

#endif
