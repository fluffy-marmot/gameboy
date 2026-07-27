#ifndef GB_IRQ
#define GB_IRQ

#include "bus.h"

typedef struct {
    uint8_t IE;
    uint8_t IF;
} gb_irq_handler_t;

gb_irq_handler_t *init_gameboy_irq(gb_bus_t *);

#endif