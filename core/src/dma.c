#include "dma.h"

// clear bit 5 of address high byte if we exceed max value
#define DMA_MAX_VALUE                           0xDF
#define DMA_MAX_VALUE_OVERFLOW_MASK             0b11101111

static gb_dma_t dma;

static uint8_t
read_bus_dma(memaddr address)
{
    if (address == MEMADDR_DMA)
        return dma.DMA;
    return UNREADABLE;
}
static void
write_bus_dma(memaddr address, uint8_t val)
{
    if (address == MEMADDR_DMA) {
        dma.DMA = val > DMA_MAX_VALUE ? val & DMA_MAX_VALUE_OVERFLOW_MASK : val;
        dma.cycles_active = GB_DMG_OAM_SIZE;
    }
}
static bus_interface_t bus_dma = { .read = read_bus_dma, .write = write_bus_dma };

void
cycle_mcycle_dma(void)
{
    dma.status = DMA_TRANSFER_INACTIVE;
    if (dma.cycles_active) {
        uint8_t byte = GB_DMG_OAM_SIZE - dma.cycles_active--;
        dma.data = dma.bus->read(((dma.DMA << 8) + byte));
        dma.bus->write(ADDR_START_OAM_MEM + byte, dma.data);
        dma.status = DMA_TRANSFER_ACTIVE;
    }
}

gb_dma_t *
init_gameboy_dma(gb_bus_t *bus)
{
    dma.bus = bus->bus_dispatcher;
    bus->interface_dma = &bus_dma;
    bus->dma = &dma;
    return &dma;
}