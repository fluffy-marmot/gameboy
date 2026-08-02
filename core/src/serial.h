#ifndef GB_SERIAL
#define GB_SERIAL

#include "_specification.h"
#include "bus.h"
#include "irq.h"

typedef struct {
    uint16_t size;
    uint16_t capacity;
    uint8_t *data;
} serial_buffer_t;

typedef struct{
    uint8_t SB;                                 // serial transfer data
    uint8_t SC;                                 // serial transfer control

    uint8_t current_bit;
    serial_buffer_t buffer;
    gb_irq_handler_t *irq;
} gb_serial_t;

gb_serial_t *init_gameboy_serial(gb_bus_t *, gb_irq_handler_t *);
void cycle_mcycle_serial(void);

#endif