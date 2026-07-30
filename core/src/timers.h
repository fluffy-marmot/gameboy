#ifndef GB_TIMERS
#define GB_TIMERS

#include "bus.h"
#include "irq.h"

typedef struct {
    uint16_t DIV;                               // Divider register
    uint8_t TIMA;                               // Timer counter register
    uint8_t TMA;                                // Timer modulo register
    uint8_t TAC;                                // Timer control register

    uint8_t timer_operation_stored;
    uint8_t tima_overflow_cycles;

    gb_irq_handler_t *irq;                      // direct access to interrupts data
} gb_timers_t;

gb_timers_t *init_gameboy_timers(gb_bus_t *, gb_irq_handler_t *);

void cycle_tcycle_timers(void);

#endif