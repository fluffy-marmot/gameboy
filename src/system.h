#ifndef GB_SYSTEM
#define GB_SYSTEM

#include "bootrom.h"
#include "bus.h"
#include "cpu.h"
#include "irq.h"
#include "ppu.h"

#define BOOT_ROM                    "bootroms/dmg_boot.bin"

typedef struct {
    gb_cpu_t *cpu;
    gb_bus_t *bus;
    gb_ppu_t *ppu;

    gb_irq_handler_t *irq;
} gb_gameboy_t;

#endif