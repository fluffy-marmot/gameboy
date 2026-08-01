#ifndef GB_JOYPAD
#define GB_JOYPAD

#include "_specification.h"
#include "bus.h"
#include "irq.h"

#include <stdbool.h>

typedef struct{
    uint8_t JOYP;                               // P1 / Joypad register

    bool BUTTON_SELECT;
    bool BUTTON_START;
    bool BUTTON_B;
    bool BUTTON_A;
    bool BUTTON_DOWN;
    bool BUTTON_UP;
    bool BUTTON_LEFT;
    bool BUTTON_RIGHT;

    bus_interface_t *bus;
    gb_irq_handler_t *irq;                      // direct access to interrupts data
} gb_joypad_t;

gb_joypad_t *init_gameboy_joypad(gb_bus_t *, gb_irq_handler_t *);

#endif