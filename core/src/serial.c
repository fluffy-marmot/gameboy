#include "_abi.h"
#include "serial.h"

#define MASK_BIT_L                              0b00000001

#define USEPINS_SC                              0b10000001
#define TRANSFER_ENABLE_BIT                     0b10000000
#define CLOCK_SELECT_BIT                        0b00000001

#define TRANSFER_ENABLED                        (serial.SC & TRANSFER_ENABLE_BIT)
#define CLOCK_SELECT                            (serial.SC & CLOCK_SELECT_BIT   )

#define CLOCK_EXTERNAL                          (CLOCK_SELECT == 0b0)
#define CLOCK_INTERNAL                          (CLOCK_SELECT == 0b1)

#define BUFFER_OUT_SIZE_INITIAL                 (64 * BYTES)
#define BUFFER_OUT_SIZE_MAX                     ( 4 * KiB)
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
        // printf("Written to SB: %d\n", val);
        serial.SB = val;
        break;
    case MEMADDR_SC:
        serial.SC = (val & USEPINS_SC) | (USEPINS_SC ^ 0xFF);
        break;
    }
}
static bus_interface_t bus_registers_serial = { .read = read_serial_reg, .write = write_serial_reg };

void
cycle_serial(void)
{
    // if (serial.buf.size == UINT16_MAX)
    //     printf("Serial buffer at max size\n");
    if (TRANSFER_ENABLED && CLOCK_INTERNAL) {
        // if we've reached max buffer size, assume client doesn't read serial and discard
        if (serial.buf.size == BUFFER_OUT_SIZE_MAX)
            serial.buf.size = 0;
        serial.buf.byte <<= 1;
        serial.buf.byte |= (serial.SB >> 7);
        serial.SB = (serial.SB << 1) | MASK_BIT_L;

        if (++serial.buf.bit == 8) {
            serial.buf.data[serial.buf.size] = serial.buf.byte;
            // printf("Serial sent out: %d\n", serial.buf.byte);
            if (++serial.buf.size == serial.buf.capacity) {
                serial.buf.capacity = MIN(BUFFER_OUT_SIZE_MAX, serial.buf.capacity * 2);
                serial.buf.data = (uint8_t *) realloc(serial.buf.data, serial.buf.capacity);
            }
            serial.buf.bit = 0;
            serial.SC ^= TRANSFER_ENABLE_BIT;
            serial.irq->IF |= INTERRUPT_BIT_SERIAL;
        }
    }
}

gb_serial_t *
init_gameboy_serial(gb_bus_t *bus, gb_irq_handler_t *irq)
{
    free(serial.buf.data);
    memset(&serial, 0, sizeof(gb_serial_t));
    serial.buf.capacity = BUFFER_OUT_SIZE_INITIAL;
    serial.buf.data = (uint8_t *) malloc(serial.buf.capacity * sizeof(uint8_t));
    serial.irq = irq;
    bus->interface_reg_serial = &bus_registers_serial;
    return &serial;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions
        
###############################################################################
############################################################################ */

// caller should read size of buf before flushing it, as it will be set to 0
uint8_t *
GB_serial_buffer_flush(void)
{
    serial.buf.size = 0;
    return serial.buf.data;

}