#include "irq.h"

#include <string.h>

#define BIT                         0b00000001

static gb_irq_handler_t irq_handler;
static uint8_t stat_interrupt_line;

// trigger STAT interrupts only on low to high transitions
static void 
update_stat_interrupt_line(uint8_t new_value)
{
    if (!stat_interrupt_line && new_value)
        irq_handler.IF |= INTERRUPT_BIT_STAT;
    stat_interrupt_line = new_value;
}

static interrupt_t
check_next_enabled_and_requested(void)
{
    for (interrupt_t i = INTERRUPT_VBLANK; i <= INTERRUPT_JOYPAD; i++)
        if (irq_handler.IE & irq_handler.IF & (BIT << i))
            return i;
    return INTERRUPT_NONE;
}

// this simply unsets the relevant IF bit and returns memory address of the handler
static memaddr
call_interrupt(interrupt_t interrupt)
{
    irq_handler.IF ^= (BIT << interrupt);
    switch (interrupt) {
    case INTERRUPT_VBLANK:          return ADDR_INTERRUPT_VBLANK;
    case INTERRUPT_STAT:            return ADDR_INTERRUPT_STAT;
    case INTERRUPT_TIMER:           return ADDR_INTERRUPT_TIMER;
    case INTERRUPT_SERIAL:          return ADDR_INTERRUPT_SERIAL;
    case INTERRUPT_JOYPAD:          return ADDR_INTERRUPT_JOYPAD;
    default:                        return 0x0000;
    }
}

static uint8_t
read_interrupt_registers(memaddr address)
{ 
    switch (address) {
    case MEMADDR_IF:                return irq_handler.IF;
    case MEMADDR_IE:                return irq_handler.IE;
    default:                        return UNREADABLE;
    }
}

static void
write_interrupt_registers(memaddr address, uint8_t value)
{
    switch (address) {
    case MEMADDR_IF:
        irq_handler.IF = value;
        break;
    case MEMADDR_IE:
        irq_handler.IE = value;
        break;
    default:
    }
}
static bus_interface_t bus_registers_interrupt = { read_interrupt_registers, write_interrupt_registers };

gb_irq_handler_t *
init_gameboy_irq(gb_bus_t *bus)
{
    memset(&irq_handler, 0, sizeof(gb_irq_handler_t));
    irq_handler.update_stat_interrupt_line = update_stat_interrupt_line;
    irq_handler.check_next_enabled_and_requested = check_next_enabled_and_requested;
    irq_handler.call_interrupt = call_interrupt;
    bus->interface_reg_interrupt = &bus_registers_interrupt;
    return &irq_handler;
}