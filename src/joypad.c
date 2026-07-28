#include "_specification.h"
#include "joypad.h"

#define USEPINS_JOYP                            0b00111111
#define WRITEABLE_JOYP                          0b00110000
#define READONLY_JOYP                           0b00001111

#define JOYP_BIT_SELECT_BUTTONS                 0b00100000
#define JOYP_BIT_SELECT_DPAD                    0b00010000
#define JOYP_BIT_START_DOWN                     0b00100000
#define JOYP_BIT_SELECT_BUTTONS                 0b00100000
#define JOYP_BIT_SELECT_BUTTONS                 0b00100000
#define JOYP_BIT_SELECT_BUTTONS                 0b00100000

static gb_joypad_t joypad;

static uint8_t read_joypad_reg(memaddr address) {
    return address == MEMADDR_JOYP ? joypad.JOYP | (USEPINS_JOYP ^ 0xFF) : UNREADABLE;
}
static void write_joypad_reg(memaddr address, uint8_t val) {
    if (address == MEMADDR_JOYP) 
        joypad.JOYP = (joypad.JOYP & READONLY_JOYP) | (val & WRITEABLE_JOYP) | (USEPINS_JOYP ^ 0xFF);
}
static bus_interface_t bus_reg_joypad = { .read = read_joypad_reg, .write = write_joypad_reg };

gb_joypad_t *
init_gameboy_joypad(gb_bus_t *bus)
{
    bus->interface_reg_joypad = &bus_reg_joypad;
    return &joypad;
}