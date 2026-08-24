#ifndef GB_SERIAL
#define GB_SERIAL

#include "_specification.h"
#include "bus.h"
#include "irq.h"

typedef struct {
    uint16_t size;
    uint16_t capacity;
    uint8_t *data;

    uint8_t byte;
    uint8_t bit;
} serial_buffer_t;

typedef struct{
    uint8_t SB;                                 // serial transfer data
    uint8_t SC;                                 // serial transfer control

    serial_buffer_t buf;
    gb_irq_handler_t *irq;
} gb_serial_t;

gb_serial_t *init_gameboy_serial(gb_bus_t *, gb_irq_handler_t *);
void cycle_serial(void);

#endif
