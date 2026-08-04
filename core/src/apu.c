#include "_abi.h"
#include "apu.h"

//////////////////////////////////////////////////////////
/* GLOBAL AUDIO REGISTERS */
//////////////////////////////////////////////////////////

// NR52 - audio master control
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

// NR51 - sound panning
#define NR51_BIT_CH4_LEFT                       0b10000000
#define NR51_BIT_CH3_LEFT                       0b01000000
#define NR51_BIT_CH2_LEFT                       0b00100000
#define NR51_BIT_CH1_LEFT                       0b00010000
#define NR51_BIT_CH4_RIGHT                      0b00001000
#define NR51_BIT_CH3_RIGHT                      0b00000100
#define NR51_BIT_CH2_RIGHT                      0b00000010
#define NR51_BIT_CH1_RIGHT                      0b00000001

// NR50 - master volume & VIN panning
#define NR50_BIT_VIN_LEFT                       0b10000000
#define NR50_BITS_LEFT_VOLUME                   0b01110000
#define NR50_BIT_VIN_RIGHT                      0b00001000
#define NR50_BITS_RIGHT_VOLUME                  0b00000111

//////////////////////////////////////////////////////////
/* CHANNEL 1 */
//////////////////////////////////////////////////////////

// NR10 - channel 1 sweep
#define USEPINS_NR10                            0b01111111

#define NR10_BITS_PACE                          0b01110000
#define NR10_BITS_DIRECTION                     0b00001000
#define NR10_BITS_INDIVIDUAL_STEP               0b00000111

// NR11 - channel 1 length timer & duty cycle
#define NR11_BITS_WAVE_DUTY                     0b11000000
#define NR11_BITS_INIT_LENGTH_TIMER             0b00111111

// NR12 - channel 1 volume & envelope
#define NR12_BITS_INITIAL_VOLUME                0b11110000
#define NR12_BIT_ENVELOPE_DIR                   0b00001000
#define NR12_BITS_SWEEP_PACE                    0b00000111

#define CH1_INITIAL_VOLUME                      ((apu.NR12 & NR12_BITS_INITIAL_VOLUME) >> 4)
#define CH1_SWEEP_PACE                          (apu.NR12 & NR12_BITS_SWEEP_PACE)

// NR13 - channel 1 period low

// NR14 - channel 1 period high & control
#define USEPINS_NR14                            0b11000111

#define NR14_BIT_TRIGGER                        0b10000000
#define NR14_BIT_LENGTH_ENABLE                  0b01000000
#define NR14_BITS_PERIOD_HIGH                   0b00000111

//////////////////////////////////////////////////////////
/* CHANNEL 2 */
//////////////////////////////////////////////////////////

// NR21 - channel 2 length timer & duty cycle
#define NR21_BITS_WAVE_DUTY                     0b11000000
#define NR21_BITS_INIT_LENGTH_TIMER             0b00111111

// NR22 - channel 2 volume & envelope
#define NR22_BITS_INITIAL_VOLUME                0b11110000
#define NR22_BIT_ENVELOPE_DIR                   0b00001000
#define NR22_BITS_SWEEP_PACE                    0b00000111

#define CH2_INITIAL_VOLUME                      ((apu.NR22 & NR22_BITS_INITIAL_VOLUME) >> 4)
#define CH2_SWEEP_PACE                          (apu.NR22 & NR22_BITS_SWEEP_PACE)

// NR23 - channel 2 period low

// NR24 - channel 2 period high & control
#define USEPINS_NR24                            0b11000111

#define NR24_BIT_TRIGGER                        0b10000000
#define NR24_BIT_LENGTH_ENABLE                  0b01000000
#define NR24_BITS_PERIOD_HIGH                   0b00000111

//////////////////////////////////////////////////////////
/* CHANNEL 3 */
//////////////////////////////////////////////////////////

// NR30 - channel 3 DAC enable
#define USEPINS_NR30                            0b10000000

#define NR30_BIT_DAC                            0b10000000

#define CH3_DAC                                 (apu.NR30 & NR30_BIT_DAC)

// NR31 - channel 3 length timer

// NR32 - channel 3 output level
#define USEPINS_NR32                            0b01100000

#define NR32_BITS_OUTPUT_LEVEL                  0b01100000

#define CH3_OUTPUT_LEVEL                        ((apu.NR32 & NR32_BITS_OUTPUT_LEVEL) >> 5)

// NR33 - channel 3 period low

// NR34 - channel 3 period high & control
#define USEPINS_NR34                            0b11000111

#define NR34_BIT_TRIGGER                        0b10000000
#define NR34_BIT_LENGTH_ENABLE                  0b01000000
#define NR34_BITS_PERIOD_HIGH                   0b00000111

//////////////////////////////////////////////////////////
/* CHANNEL 4 */
//////////////////////////////////////////////////////////

// NR41 - channel 4 length timer
#define USEPINS_NR41                            0b00111111

#define NR41_BITS_LENGTH_TIMER                  0b00111111

#define CH4_LENGTH_TIMER                        (apu.NR41 & NR41_BITS_LENGTH_TIMER)

// NR42 - channel 4 volume & envelope
#define NR42_BITS_INITIAL_VOLUME                0b11110000
#define NR42_BIT_ENVELOPE_DIR                   0b00001000
#define NR42_BITS_SWEEP_PACE                    0b00000111

#define CH4_INITIAL_VOLUME                      ((apu.NR42 & NR42_BITS_INITIAL_VOLUME) >> 4)
#define CH4_SWEEP_PACE                          (apu.NR42 & NR42_BITS_SWEEP_PACE)

// NR43 - channel 4 frequency & randomness
#define NR43_BITS_CLOCK_SHIFT                   0b11110000
#define NR43_BIT_LFSR_WIDTH                     0b00001000
#define NR43_BITS_CLOCK_DIVIDER                 0b00000111

#define CH4_CLOCK_SHIFT                         ((apu.NR43 & NR43_BITS_CLOCK_SHIFT) >> 4)
#define CH4_LFSR_WIDTH                          (apu.NR43 & NR43_BIT_LFSR_WIDTH)
#define CH4_CLOCK_DIVIDER                       (apu.NR43 & NR43_BITS_CLOCK_DIVIDER)

// NR44 - channel 4 period high & control
#define USEPINS_NR44                            0b11000000

#define NR44_BIT_TRIGGER                        0b10000000
#define NR44_BIT_LENGTH_ENABLE                  0b01000000


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