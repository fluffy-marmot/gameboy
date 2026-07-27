#ifndef GB_SYSTEM
#define GB_SYSTEM

#include "bootrom.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "irq.h"
#include "ppu.h"

typedef struct {
    gb_cpu_t *cpu;
    gb_bus_t *bus;
    gb_ppu_t *ppu;

    gb_irq_handler_t *irq;
} gb_gameboy_t;

// ABI
void set_post_boot_state(void);

// ABI

#endif