#include "_abi.h"
#include "serial.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define MASK_BIT_L                              0b00000001

#define USEPINS_SC                              0b10000001
#define TRANSFER_ENABLE_BIT                     0b10000000
#define CLOCK_SELECT_BIT                        0b00000001

#define TRANSFER_ENABLED                        (serial.SC & TRANSFER_ENABLE_BIT)
#define CLOCK_SELECT                            (serial.SC & CLOCK_ENABLE_BIT   )

#define BUFFER_OUT_INITIAL_SIZE                 (64 * BYTES)
#define MIN(x, y)                               ((x < y) ? (x) : (y))

static gb_serial_t serial;

static uint8_t
read_serial_reg(memaddr address)
{
    switch (address) {
    case MEMADDR_SB:                            return serial.SB;
    case MEMADDR_SC:                            return (serial.SC & USEPINS_SC) | (USEPINS_SC ^ 0xFF);
    default:                                    return UNREADABLE;
    }       
}
static void
write_serial_reg(memaddr address, uint8_t val)
{
    switch (address) {
    case MEMADDR_SB:
        serial.SB = val;
        break;
    case MEMADDR_SC:
        serial.SC = (val & USEPINS_SC) | (USEPINS_SC ^ 0xFF);
        break;
    }
}
static bus_interface_t bus_registers_serial = { .read = read_serial_reg, .write = write_serial_reg };
// TODO: also add ctypes for serial

void
cycle_mcycle_serial(void)
{
    if (TRANSFER_ENABLED && serial.buffer.size < UINT16_MAX) {
        serial.buffer.data[serial.buffer.size] <<= 1;
        serial.buffer.data[serial.buffer.size] |= (serial.SB >> 7);
        serial.SB = (serial.SB << 1) | (0xFF & MASK_BIT_L);

        if (++serial.current_bit == 8) {
            if (++serial.buffer.size == serial.buffer.capacity && serial.buffer.capacity < UINT16_MAX) {
                serial.buffer.capacity = MIN(UINT16_MAX, serial.buffer.capacity * 2);
                serial.buffer.data = (uint8_t *) realloc(serial.buffer.data, serial.buffer.capacity);
            }
            serial.current_bit = 0;
            serial.SC ^= TRANSFER_ENABLE_BIT;
            serial.irq->IF |= INTERRUPT_BIT_SERIAL;
        }
    }
}

gb_serial_t *
init_gameboy_serial(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    free(serial.buffer.data);
    memset(&serial, 0, sizeof(gb_serial_t));
    serial.buffer.capacity = BUFFER_OUT_INITIAL_SIZE;
    serial.buffer.data = (uint8_t *) malloc(serial.buffer.capacity * sizeof(uint8_t));
    serial.irq = irq;
    bus->interface_reg_serial = &bus_registers_serial;
    return &serial;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions
        
###############################################################################
############################################################################ */

// caller should read size of buffer before flushing it, as it will be set to 0
uint8_t *
GB_serial_buffer_flush(void)
{
    serial.buffer.size = 0;
    return serial.buffer.data;

}