#ifndef GB_SYSTEM
#define GB_SYSTEM

#include "bus.h"
#include "cpu.h"

#define GB_DISPLAY_WIDTH            160
#define GB_DISPLAY_HEIGHT           144

typedef struct {
    gb_cpu_t *cpu;
    uint8_t *mem;       // TODO
} gb_gameboy_t;

#endif