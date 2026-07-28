#ifndef GB_JOYPAD
#define GB_JOYPAD

#include "bus.h"

typedef enum {
    PRESSED = 0,
    
} button_state_t;

typedef struct{
    uint8_t JOYP;                               // P1 / Joypad register



    bus_interface_t *bus;
} gb_joypad_t;

gb_joypad_t *init_gameboy_joypad(gb_bus_t *);

#endif