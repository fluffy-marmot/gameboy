#ifndef GB_SYSTEM
#define GB_SYSTEM

#include "bootrom.h"
#include "bus.h"
#include "cpu.h"

#define GB_DISPLAY_WIDTH            160
#define GB_DISPLAY_HEIGHT           144

#define BOOT_ROM                    "bootroms/dmg_boot.bin"

typedef struct {
    gb_cpu_t *cpu;
    gb_bus_t *bus;
} gb_gameboy_t;

#endif