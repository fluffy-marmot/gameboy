#include "_specification.h"
#include "joypad.h"

#include <stdio.h>

#define USEPINS_JOYP                            0b00111111
#define WRITEABLE_JOYP                          0b00110000
#define READONLY_JOYP                           0b00001111

#define JOYP_BIT_SELECT_BUTTONS                 0b00100000
#define JOYP_BIT_SELECT_DPAD                    0b00010000
#define JOYP_BIT_START_DOWN                     0b00001000
#define JOYP_BIT_SELECT_UP                      0b00000100
#define JOYP_BIT_B_LEFT                         0b00000010
#define JOYP_BIT_A_RIGHT                        0b00000001

#define JOYP_SELECT_BUTTONS                     (joypad.JOYP & JOYP_BIT_SELECT_BUTTONS)
#define JOYP_SELECT_DPAD                        (joypad.JOYP & JOYP_BIT_SELECT_DPAD)

#define PRESSED                                 0
#define SELECTED                                0

static gb_joypad_t joypad;

static void
update_joyp_register(void)
{
    uint8_t prev_state = joypad.JOYP;

    joypad.JOYP |= WRITEABLE_JOYP ^ 0xFF;
    if (JOYP_BIT_SELECT_BUTTONS  == SELECTED) {
        if (joypad.BUTTON_START  == PRESSED)    joypad.JOYP &= JOYP_BIT_START_DOWN ^ 0xFF;
        if (joypad.BUTTON_SELECT == PRESSED)    joypad.JOYP &= JOYP_BIT_SELECT_UP  ^ 0xFF;
        if (joypad.BUTTON_B      == PRESSED)    joypad.JOYP &= JOYP_BIT_B_LEFT     ^ 0xFF;
        if (joypad.BUTTON_A      == PRESSED)    joypad.JOYP &= JOYP_BIT_A_RIGHT    ^ 0xFF;
    }
    if (JOYP_BIT_SELECT_DPAD     == SELECTED) {
        if (joypad.BUTTON_DOWN   == PRESSED)    joypad.JOYP &= JOYP_BIT_START_DOWN ^ 0xFF;
        if (joypad.BUTTON_UP     == PRESSED)    joypad.JOYP &= JOYP_BIT_SELECT_UP  ^ 0xFF;
        if (joypad.BUTTON_LEFT   == PRESSED)    joypad.JOYP &= JOYP_BIT_B_LEFT     ^ 0xFF;
        if (joypad.BUTTON_RIGHT  == PRESSED)    joypad.JOYP &= JOYP_BIT_A_RIGHT    ^ 0xFF;
    }

    if (
        ((prev_state & JOYP_BIT_START_DOWN) && !(joypad.JOYP & JOYP_BIT_START_DOWN)) ||
        ((prev_state & JOYP_BIT_SELECT_UP)  && !(joypad.JOYP & JOYP_BIT_SELECT_UP))  ||
        ((prev_state & JOYP_BIT_B_LEFT)     && !(joypad.JOYP & JOYP_BIT_B_LEFT))     ||
        ((prev_state & JOYP_BIT_A_RIGHT)    && !(joypad.JOYP & JOYP_BIT_A_RIGHT))
    ) {
        joypad.irq->IF |= INTERRUPT_BIT_JOYPAD;
    }
}

static uint8_t
read_joypad_reg(memaddr address)
{
    if (address == MEMADDR_JOYP) {
        return joypad.JOYP | (USEPINS_JOYP ^ 0xFF);
    } else
        return UNREADABLE;
}
static void
write_joypad_reg(memaddr address, uint8_t val)
{
    if (address == MEMADDR_JOYP) {
        joypad.JOYP = (joypad.JOYP & READONLY_JOYP) | (val & WRITEABLE_JOYP) | (USEPINS_JOYP ^ 0xFF);
        update_joyp_register();
    }

    printf("0x%X  0x%X\n", val, joypad.JOYP);

}
static bus_interface_t bus_reg_joypad = { .read = read_joypad_reg, .write = write_joypad_reg };

void
update_joypad(bool start, bool select, bool b, bool a, bool down, bool up, bool left, bool right)
{
    joypad.BUTTON_START  = start;
    joypad.BUTTON_SELECT = select;
    joypad.BUTTON_B      = b;
    joypad.BUTTON_A      = a;
    joypad.BUTTON_DOWN   = down;
    joypad.BUTTON_UP     = up;
    joypad.BUTTON_LEFT   = left;
    joypad.BUTTON_RIGHT  = right;
    update_joyp_register();
    // printf("0x%X\n", joypad.JOYP);
    // printf("%d%d%d%d%d%d%d%d\n", start, select, b, a, down, up, left, right);
}

gb_joypad_t *
init_gameboy_joypad(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    joypad.irq = irq;
    bus->interface_reg_joypad = &bus_reg_joypad;
    return &joypad;
}