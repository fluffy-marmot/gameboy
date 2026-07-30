#ifndef GB_DMA
#define GB_DMA

#include "_specification.h"
#include "bus.h"

typedef enum {
    DMA_TRANSFER_INACTIVE,
    DMA_TRANSFER_ACTIVE
} dma_status_t;

typedef struct gb_dma{
    uint8_t DMA;                                // DMA control register

    uint8_t cycles_active;
    uint8_t data;
    dma_status_t status;

    bus_interface_t *bus;
} gb_dma_t;

gb_dma_t *init_gameboy_dma(gb_bus_t *);
void cycle_mcycle_dma(void);

#endif