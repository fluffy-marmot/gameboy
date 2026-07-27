#include "irq.h"
#include "specification.h"


static gb_irq_handler_t irq_handler;

static uint8_t
read_interrupt_registers(memaddr address)
{ 
    switch (address) {
    case MEMADDR_IF:
        return irq_handler.IF;
    case MEMADDR_IE:
        return irq_handler.IE;
    default:
        return UNREADABLE;
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
    bus->interface_reg_interrupt = &bus_registers_interrupt;
    return &irq_handler;
}