#ifndef GB_SYSTEM
#define GB_SYSTEM

#include "bootrom.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "dma.h"
#include "irq.h"
#include "joypad.h"
#include "ppu.h"
#include "timers.h"

typedef struct {
    gb_cpu_t *cpu;
    gb_bus_t *bus;
    gb_dma_t *dma;
    gb_ppu_t *ppu;
    gb_joypad_t *joypad;
    gb_timers_t *timers;

    gb_irq_handler_t *irq;
} gb_gameboy_t;

// ABI
void set_post_boot_state(void);
void emulate_frame(void);

// ABI

#endif