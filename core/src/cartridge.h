#ifndef GB_CARTRIDGE
#define GB_CARTRIDGE

#include "_specification.h"
#include "bus.h"

typedef struct {
    struct {
        mbc_type_t     mbc_type;
        mbc_rom_size_t rom_size;
        mbc_ram_size_t ram_size;
    } header;

    size_t   rom_size_bytes;
    uint8_t *rom;
} gb_cartridge_t;

gb_cartridge_t *init_cartridge(gb_bus_t *);

#endif