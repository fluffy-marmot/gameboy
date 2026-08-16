#include "dma.h"

// clear bit 5 of address high byte if we exceed max value
#define DMA_MAX_VALUE                           0xDF
#define DMA_MAX_VALUE_OVERFLOW_MASK             0b11011111

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
        dma.scheduled = 1;
    }
}
static bus_interface_t bus_dma = { .read = read_bus_dma, .write = write_bus_dma };

void
cycle_mcycle_dma(void)
{
    dma.status = DMA_INACTIVE;

    if (dma.cycles_active) {
        uint8_t byte = GB_DMG_OAM_SIZE - dma.cycles_active--;
        dma.data = dma.bus->read(((dma.DMA_latched << 8) + byte));
        dma.bus->write(ADDR_START_OAM_MEM + byte, dma.data);
        if (ADDR_START_VRAM <= (dma.DMA << 8) && (dma.DMA << 8) <= ADDR_END_VRAM)
            dma.status = DMA_ACTIVE_BUS_VRAM;
        else
            dma.status = DMA_ACTIVE_BUS_EXTERNAL;
    }

    if (dma.scheduled && dma.scheduled--) {
        dma.cycles_active = GB_DMG_OAM_SIZE;
        dma.DMA_latched = dma.DMA;
    }
}

gb_dma_t *
init_gameboy_dma(gb_bus_t *bus)
{
    memset(&dma, 0, sizeof(gb_dma_t));
    dma.bus = bus->bus_dispatcher_dma;
    bus->interface_reg_dma = &bus_dma;
    bus->dma = &dma;
    return &dma;
}