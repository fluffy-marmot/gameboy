#include "_abi.h"
#include "apu.h"

#define USEPINS_NR52                            0b10001111
#define WRITEABLE_NR52                          0b10000000
#define READONLY_NR52                           0b00001111

#define NR52_BIT_APU_ENABLE                     0b10000000
#define NR52_BIT_CH1                            0b00000001
#define NR52_BIT_CH2                            0b00000010
#define NR52_BIT_CH3                            0b00000100
#define NR52_BIT_CH4                            0b00001000

#define APU_ENABLED                             (apu.NR52 & NR52_BIT_APU_ENABLE)
#define CH1_ON                                  (apu.NR52 & NR52_BIT_CH1)
#define CH2_ON                                  (apu.NR52 & NR52_BIT_CH2)
#define CH3_ON                                  (apu.NR52 & NR52_BIT_CH3)
#define CH4_ON                                  (apu.NR52 & NR52_BIT_CH4)

gb_apu_t apu;

static uint8_t 
read_apu_reg (memaddr address)
{
    switch (address) {
    case MEMADDR_NR52:                          return apu.NR52 | (USEPINS_NR52 ^ 0xFF);
    case MEMADDR_NR51:                          return apu.NR51;
    default:                                    return UNREADABLE;
    }
}

static void
write_apu_reg(memaddr address, uint8_t val)
{
    switch (address) {
    case MEMADDR_NR52:
        // TODO turning off clears all APU registers and makes them readonly, except for NR52
        apu.NR52   = (WRITEABLE_NR52 & val) | (READONLY_NR52 & apu.NR52) | (USEPINS_NR52 ^ 0xFF);
        break;
    case MEMADDR_NR51:                          apu.NR51 = val;                                     break;
    }
}

static bus_interface_t bus_registers_apu =  { .read = read_apu_reg, .write = write_apu_reg };

gb_apu_t *
init_gameboy_apu(gb_bus_t *bus)
{
    bus->interface_reg_apu = &bus_registers_apu;
    return &apu;
}

// TODO DIV-APU counter - increased on every falling edge of DIV bit 4
// Affects: Envelope sweep (8 DIV-APU ticks), sound length (2 ticks), CH1 freq. sweep (4 ticks)