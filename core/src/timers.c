#include "timers.h"

#include <string.h>

#define USEPINS_TAC                             0b00000111

#define TAC_ENABLE_BIT                          0b00000100
#define TAC_CLOCK_SELECT_BITS                   0b00000011

#define TAC_ENABLED                             (timers.TAC & TAC_ENABLE_BIT)
#define TAC_CLOCK_SELECT                        (timers.TAC & TAC_CLOCK_SELECT_BITS)

static const uint16_t DIV_BIT_USED[] = {        0b1000000000,
                                                0b0000001000,
                                                0b0000100000,
                                                0b0010000000,
};

static gb_timers_t timers;

static uint8_t
read_timers_reg(memaddr address)
{
    switch (address) {
    case MEMADDR_DIV:                           return timers.DIV >> 8;
    case MEMADDR_TIMA:                          return timers.TIMA;
    case MEMADDR_TMA:                           return timers.TMA;
    case MEMADDR_TAC:                           return timers.TAC | (USEPINS_TAC ^ 0xFF);
    default:                                    return 0xFF;
    }       
}
static void
write_timers_reg(memaddr address, uint8_t val)
{
    switch (address) {
    case MEMADDR_DIV:
        timers.DIV  = 0x0000;                     
        break;
    case MEMADDR_TMA:                           
        timers.TMA  = val;                        
        break;
    case MEMADDR_TAC:                           
        timers.TAC  = val | (USEPINS_TAC ^ 0xFF); 
        break;
    case MEMADDR_TIMA:
        if (timers.tima_overflow_cycles != 1)   timers.TIMA = val;
        if (timers.tima_overflow_cycles > 1)    timers.tima_overflow_cycles = 0;
        break;
    }
}
static bus_interface_t bus_registers_timers = { .read = read_timers_reg, .write = write_timers_reg };

void
cycle_tcycle_timers(void)
{
    timers.DIV++;
    uint8_t timer_operation = TAC_ENABLED && (timers.DIV & DIV_BIT_USED[TAC_CLOCK_SELECT]);

    if (timers.tima_overflow_cycles) {
        if (--timers.tima_overflow_cycles == 0) {
            timers.TIMA = timers.TMA;
            timers.irq->IF |= INTERRUPT_BIT_TIMER;
        }
    // check for falling edge to increment TIMA timer
    } else if (timers.timer_operation_stored && !timer_operation) {
        if (++timers.TIMA == 0x00) {
            timers.tima_overflow_cycles = 4;
        }
    }
    timers.timer_operation_stored = timer_operation;
}

gb_timers_t *
init_gameboy_timers(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    memset(&timers, 0, sizeof(gb_timers_t));
    timers.irq = irq;
    bus->interface_reg_timers = &bus_registers_timers;
    return &timers;
}