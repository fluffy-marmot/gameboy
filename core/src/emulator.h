#ifndef GB_EMULATOR
#define GB_EMULATOR

#include "_specification.h"
#include "apu.h"
#include "bootrom.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "dma.h"
#include "irq.h"
#include "joypad.h"
#include "ppu.h"
#include "serial.h"
#include "timers.h"

typedef struct {
    gb_cpu_t *cpu;
    gb_bus_t *bus;
    gb_dma_t *dma;
    gb_ppu_t *ppu;
    gb_apu_t *apu;
    gb_joypad_t *joypad;
    gb_serial_t *serial;
    gb_timers_t *timers;

    gb_irq_handler_t *irq;
    gb_cartridge_t *cartridge;
} gb_gameboy_t;

GB_ABI extern gb_gameboy_t gb;

#endif
