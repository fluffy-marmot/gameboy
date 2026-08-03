#ifndef GB_CARTRIDGE
#define GB_CARTRIDGE

#include "_specification.h"
#include "bus.h"

typedef struct {
    uint8_t *rom;
    uint8_t *ram;
    size_t rom_size;
    size_t ram_size;
} cart_data_t;

typedef struct {
    struct {
        mbc_type_t     mbc_type;
        mbc_rom_size_t rom_size_id;
        mbc_ram_size_t ram_size_id;
    } header;

    cart_data_t data;
    bus_interface_t *mbc_bus;
} gb_cartridge_t;

gb_cartridge_t *init_cartridge(gb_bus_t *);

#endif