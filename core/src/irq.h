#ifndef GB_IRQ
#define GB_IRQ

#include "_specification.h"
#include "bus.h"

#define INTERRUPT_BIT_VBLANK        0b00000001
#define INTERRUPT_BIT_STAT          0b00000010
#define INTERRUPT_BIT_TIMER         0b00000100
#define INTERRUPT_BIT_SERIAL        0b00001000
#define INTERRUPT_BIT_JOYPAD        0b00010000

typedef enum {
    INTERRUPT_VBLANK,
    INTERRUPT_STAT,
    INTERRUPT_TIMER,
    INTERRUPT_SERIAL,
    INTERRUPT_JOYPAD,
    INTERRUPT_NONE
} interrupt_t;

typedef struct {
    uint8_t IE;
    uint8_t IF;

    interrupt_t (*check_next_enabled_and_requested) (void);
    memaddr     (*call_interrupt)                   (interrupt_t);
    void        (*update_stat_interrupt_line)       (uint8_t);
} gb_irq_handler_t;

gb_irq_handler_t *init_gameboy_irq(gb_bus_t *);

#endif
