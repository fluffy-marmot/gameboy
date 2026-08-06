#include "_abi.h"
#include "apu.h"

//////////////////////////////////////////////////////////
/* GLOBAL AUDIO REGISTERS */
//////////////////////////////////////////////////////////

// NR52 - audio master control
#define USEPINS_NR52                            0b10001111
#define WRITEABLE_NR52                          0b10000000
#define READONLY_NR52                           0b00001111

#define ENABLE_CH(ch)                           (apu.NR52 |=         (1 << ((ch) - 1)))
#define DISABLE_CH(ch)                          (apu.NR52 &= (0xFF ^ (1 << ((ch) - 1))))

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

#define CH1_MAX_FREQUENCY                       0x7FF

// NR10 - channel 1 sweep
#define USEPINS_NR10                            0b01111111

#define NR10_BITS_SWEEP_PACE                    0b01110000
#define NR10_BIT_SWEEP_DIR                      0b00001000
#define NR10_BITS_SWEEP_SHIFT                   0b00000111

#define CH1_SWEEP_PACE                          ((apu.NR10 & NR10_BITS_SWEEP_PACE) >> 4)
#define CH1_SWEEP_DIR_UP                        (apu.NR10 & NR10_BIT_SWEEP_DIR)
#define CH1_SWEEP_SHIFT                         (apu.NR10 & NR10_BITS_SWEEP_SHIFT)

// NR11 - channel 1 length timer & duty cycle
#define READABLE_NR11                           0b11000000

#define NR11_BITS_WAVE_DUTY                     0b11000000
#define NR11_BITS_INIT_LENGTH_TIMER             0b00111111

#define CH1_WAVE_DUTY                           ((apu.NR11 & NR11_BITS_WAVE_DUTY) >> 6)
#define CH1_INIT_LENGTH_TIMER                   (apu.NR11 & NR11_BITS_INIT_LENGTH_TIMER)

// NR13 - channel 1 period low

// NR14 - channel 1 period high & control
#define USEPINS_NR14                            0b11000111

#define NR14_BIT_TRIGGER                        0b10000000
#define NR14_BIT_LENGTH_ENABLE                  0b01000000
#define NR14_BITS_PERIOD_HIGH                   0b00000111

#define CH1_PERIOD                              (((apu.NR14 & NR14_BITS_PERIOD_HIGH) << 8) | apu.NR13)
#define CH1_LENGTH_TIMER_ENABLED                (apu.NR14 & NR14_BIT_LENGTH_ENABLE)
#define CH1_TRIGGER                             (apu.NR14 & NR14_BIT_TRIGGER)

//////////////////////////////////////////////////////////
/* CHANNEL 2 */
//////////////////////////////////////////////////////////

// NR21 - channel 2 length timer & duty cycle
#define READABLE_NR21                           0b11000000

#define NR21_BITS_WAVE_DUTY                     0b11000000
#define NR21_BITS_INIT_LENGTH_TIMER             0b00111111

#define CH2_WAVE_DUTY                           ((apu.NR21 & NR21_BITS_WAVE_DUTY) >> 6)
#define CH2_INIT_LENGTH_TIMER                   (apu.NR21 & NR21_BITS_INIT_LENGTH_TIMER)

// NR23 - channel 2 period low

// NR24 - channel 2 period high & control
#define USEPINS_NR24                            0b11000111

#define NR24_BIT_TRIGGER                        0b10000000
#define NR24_BIT_LENGTH_ENABLE                  0b01000000 
#define NR24_BITS_PERIOD_HIGH                   0b00000111

#define CH2_PERIOD                              (((apu.NR24 & NR24_BITS_PERIOD_HIGH) << 8) | apu.NR23)
#define CH2_LENGTH_TIMER_ENABLED                (apu.NR24 & NR24_BIT_LENGTH_ENABLE)
#define CH2_TRIGGER                             (apu.NR24 & NR24_BIT_TRIGGER)

//////////////////////////////////////////////////////////
/* CHANNEL 3 */
//////////////////////////////////////////////////////////

// NR30 - channel 3 DAC enable
#define USEPINS_NR30                            0b10000000

#define NR30_BIT_DAC                            0b10000000

#define CH3_DAC                                 (apu.NR30 & NR30_BIT_DAC)

// NR31 - channel 3 length timer

#define CH3_INIT_LENGTH_TIMER                   (apu.NR31)

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

#define CH3_LENGTH_TIMER_ENABLED                (apu.NR34 & NR34_BIT_LENGTH_ENABLE)
#define CH3_TRIGGER                             (apu.NR34 & NR34_BIT_TRIGGER)

//////////////////////////////////////////////////////////
/* CHANNEL 4 */
//////////////////////////////////////////////////////////

// NR41 - channel 4 length timer
#define USEPINS_NR41                            0b00111111

#define NR41_BITS_LENGTH_TIMER                  0b00111111

#define CH4_INIT_LENGTH_TIMER                   (apu.NR41 & NR41_BITS_LENGTH_TIMER)

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

#define CH4_LENGTH_TIMER_ENABLED                (apu.NR44 & NR44_BIT_LENGTH_ENABLE)
#define CH4_TRIGGER                             (apu.NR44 & NR44_BIT_TRIGGER)

//////////////////////////////////////////////////////////
/* OTHER / COMMON */
//////////////////////////////////////////////////////////

#define SHORT_TIMER                             64
#define LONG_TIMER                              256

#define MASK_BIT_L                              0b00000001
#define READABLE_NRx4                           0b01000000

// Envelope stuff for ch 1, 2, 4
#define BITMASK_ENVELOPE_INITIAL_VOLUME         0b11110000
#define BITMASK_ENVELOPE_DIR                    0b00001000
#define BITMASK_ENVELOPE_PACE                   0b00000111

#define ENVELOPE_INITIAL_VOLUME(env_reg)        ((*(env_reg) & BITMASK_ENVELOPE_INITIAL_VOLUME) >> 4)
#define ENVELOPE_DIR_UP(env_reg)                (*(env_reg) & BITMASK_ENVELOPE_DIR)
#define ENVELOPE_PACE(env_reg)                  (*(env_reg) & BITMASK_ENVELOPE_PACE)

static const uint8_t DUTY_WAVEFORM[4] = {
    [0b00] = 0b00000001,                        // 12.5 % ratio
    [0b01] = 0b00000011,                        // 25.0 % ratio
    [0b10] = 0b00001111,                        // 50.0 % ratio
    [0b11] = 0b11111100                         // 75.0 % ratio
};

static const uint8_t CH4_VOLUME_SHIFT[4] = {
    [0b00] = 0b00000100,
    [0b01] = 0b00000000,
    [0b10] = 0b00000001,
    [0b11] = 0b00000010,
};

static const float DAC[16] = {
    [0x0] = 1.0f - (2.0f *  0.0f) / 15.0f,
    [0x1] = 1.0f - (2.0f *  1.0f) / 15.0f,
    [0x2] = 1.0f - (2.0f *  2.0f) / 15.0f,
    [0x3] = 1.0f - (2.0f *  3.0f) / 15.0f,
    [0x4] = 1.0f - (2.0f *  4.0f) / 15.0f,
    [0x5] = 1.0f - (2.0f *  5.0f) / 15.0f,
    [0x6] = 1.0f - (2.0f *  6.0f) / 15.0f,
    [0x7] = 1.0f - (2.0f *  7.0f) / 15.0f,
    [0x8] = 1.0f - (2.0f *  8.0f) / 15.0f,
    [0x9] = 1.0f - (2.0f *  9.0f) / 15.0f,
    [0xA] = 1.0f - (2.0f * 10.0f) / 15.0f,
    [0xB] = 1.0f - (2.0f * 11.0f) / 15.0f,
    [0xC] = 1.0f - (2.0f * 12.0f) / 15.0f,
    [0xD] = 1.0f - (2.0f * 13.0f) / 15.0f,
    [0xE] = 1.0f - (2.0f * 14.0f) / 15.0f,
    [0xF] = 1.0f - (2.0f * 15.0f) / 15.0f,
};

static gb_apu_t apu;

static void
cycle_envelope(envelope_data_t *env)
{
    if (ENVELOPE_PACE(env->reg) == 0)    return;
    if (env->timer && --env->timer != 0) return;

    // if an envelope pace is set and decremented timer just reached 0:
    env->timer = ENVELOPE_PACE(env->reg);
    if (ENVELOPE_DIR_UP(env->reg) == 1 && env->volume < 0xF)
        env->volume++;
    else if (ENVELOPE_DIR_UP(env->reg) == 0 && env->volume > 0x0)
        env->volume--;
}

static uint16_t
calculate_sweep_frequency_and_check_overflow_ch1(void)
{
    uint16_t new_frequency = apu.ch1.sweep.shadow_frequency >> CH1_SWEEP_SHIFT;
    new_frequency = apu.ch1.sweep.shadow_frequency + new_frequency * (CH1_SWEEP_DIR_UP ? 1 : -1);
    if (new_frequency > CH1_MAX_FREQUENCY)
        DISABLE_CH(1);
    return new_frequency;
}

static void
cycle_sweep_ch1(void)
{
    if (apu.ch1.sweep.timer && --apu.ch1.sweep.timer) return;

    apu.ch1.sweep.timer = CH1_SWEEP_PACE ? CH1_SWEEP_PACE : 8;
    if (apu.ch1.sweep.enabled && CH1_SWEEP_PACE) {
        uint16_t new_frequency = calculate_sweep_frequency_and_check_overflow_ch1();
        if (new_frequency <= CH1_MAX_FREQUENCY && CH1_SWEEP_SHIFT) {
            apu.NR13 = (uint8_t) new_frequency;
            apu.NR14 = (apu.NR14 & (NR14_BITS_PERIOD_HIGH ^ 0xFF)) | (new_frequency >> 8);
            apu.ch1.sweep.shadow_frequency = new_frequency;
            // performs overflow check for new values and possibly disables the channel preemptively
            calculate_sweep_frequency_and_check_overflow_ch1();
        }
    }
}

static void
cycle_waveduty(waveduty_data_t *waveduty, uint16_t ch_period)
{
    if (waveduty->timer == 0) {
        waveduty->timer = (2048 - ch_period) * 4;
        waveduty->cycle = (waveduty->cycle + 1) % 8;
    }
}

static void
cycle_length_timers(void)
{
    if (CH1_LENGTH_TIMER_ENABLED && (--apu.ch1.len_timer == 0))     DISABLE_CH(1);
    if (CH2_LENGTH_TIMER_ENABLED && (--apu.ch2.len_timer == 0))     DISABLE_CH(2);
    if (CH3_LENGTH_TIMER_ENABLED && (--apu.ch3.len_timer == 0))     DISABLE_CH(3);
    if (CH4_LENGTH_TIMER_ENABLED && (--apu.ch4.len_timer == 0))     DISABLE_CH(4);
}

static void
trigger_envelope(envelope_data_t *env)
{
    // TODO envelope pace of 0 also treated as 8 like sweep? (pandocs)
    env->timer = ENVELOPE_PACE(env->reg);
    env->volume = ENVELOPE_INITIAL_VOLUME(env->reg);
}

static void
trigger_ch1(void)
{
    // enable
    ENABLE_CH(1);
    // length timer
    if (apu.ch1.len_timer == 0)
        apu.ch1.len_timer = SHORT_TIMER - CH1_INIT_LENGTH_TIMER;
    // envelope
    trigger_envelope(&apu.ch1.envelope);
    // sweep
    apu.ch1.sweep.shadow_frequency = CH1_PERIOD;
    apu.ch1.sweep.timer = CH1_SWEEP_PACE ? CH1_SWEEP_PACE : 8;
    apu.ch1.sweep.enabled = CH1_SWEEP_PACE | CH1_SWEEP_SHIFT;
    if (CH1_SWEEP_SHIFT)
        calculate_sweep_frequency_and_check_overflow_ch1();
}

static void
trigger_ch2(void)
{
    // enable
    ENABLE_CH(2);
    // length timer
    if (apu.ch2.len_timer == 0)
        apu.ch2.len_timer = SHORT_TIMER - CH2_INIT_LENGTH_TIMER;
    // envelope
    trigger_envelope(&apu.ch2.envelope);
}

static void
trigger_ch3(void)
{
    // enable
    ENABLE_CH(3);
    // long length timer
    if (apu.ch3.len_timer == 0)
        apu.ch3.len_timer = LONG_TIMER - CH3_INIT_LENGTH_TIMER;
}

static void
trigger_ch4(void)
{
    // enable
    ENABLE_CH(4);
    // length timer
    if (apu.ch4.len_timer == 0)
        apu.ch4.len_timer = SHORT_TIMER - CH4_INIT_LENGTH_TIMER;
}

stereo_sample_t
audio_sample(void)
{
    float left = 0.0;
    float right = 0.0;

    digital ch1_dig = 0x0;
    if (CH2_ON) {
        uint8_t ch1_vol = apu.ch1.envelope.volume;
        ch1_dig = ((DUTY_WAVEFORM[CH1_WAVE_DUTY] >> apu.ch1.waveduty.cycle) & MASK_BIT_L) * ch1_vol;
    }
    analog ch1 = DAC[ch1_dig];

}

static uint8_t 
read_apu_reg (memaddr address)
{
    // TODO - does this need to be blocked if in some particular state?
    /* On monochrome consoles, wave RAM can only be accessed on the same cycle that CH3 does. Otherwise, reads
    return $FF, and writes are ignored. */
    if (ADDR_RANGE(ADDR_START_WAVERAM, ADDR_END_WAVERAM))
        return apu.ch3.waveram[address - ADDR_START_WAVERAM];

    switch (address) {
    case MEMADDR_NR52:                          return apu.NR52 | (USEPINS_NR52 ^ 0xFF);
    case MEMADDR_NR51:                          return apu.NR51;
    case MEMADDR_NR50:                          return apu.NR50;

    case MEMADDR_NR10:                          return apu.NR10 | (USEPINS_NR10 ^ 0xFF);
    case MEMADDR_NR11:                          return apu.NR11 | (READABLE_NR11 ^ 0xFF);
    case MEMADDR_NR12:                          return apu.NR12;
    case MEMADDR_NR13:                          return UNREADABLE;
    case MEMADDR_NR14:                          return apu.NR14 | (READABLE_NRx4 ^ 0xFF);

    case MEMADDR_NR21:                          return apu.NR21 | (READABLE_NR21 ^ 0xFF);
    case MEMADDR_NR22:                          return apu.NR22;
    case MEMADDR_NR23:                          return UNREADABLE;
    case MEMADDR_NR24:                          return apu.NR24 | (READABLE_NRx4 ^ 0xFF);

    case MEMADDR_NR30:                          return apu.NR30 | (USEPINS_NR30 ^ 0xFF);
    case MEMADDR_NR31:                          return UNREADABLE;
    case MEMADDR_NR32:                          return apu.NR32 | (USEPINS_NR32 ^ 0xFF);
    case MEMADDR_NR33:                          return UNREADABLE;
    case MEMADDR_NR34:                          return apu.NR34 | (READABLE_NRx4 ^ 0xFF);

    case MEMADDR_NR41:                          return UNREADABLE;
    case MEMADDR_NR42:                          return apu.NR42;
    case MEMADDR_NR43:                          return apu.NR43;
    case MEMADDR_NR44:                          return apu.NR44 | (READABLE_NRx4 ^ 0xFF);
    default:                                    return UNREADABLE;
    }
}

static void
write_apu_reg(memaddr address, uint8_t val)
{
    // if APU is powered off, all other registers are read only
    if (!APU_ENABLED && address != MEMADDR_NR52)
        return;
    if (ADDR_RANGE(ADDR_START_WAVERAM, ADDR_END_WAVERAM)) {
        if (!CH3_ON)
            apu.ch3.waveram[address - ADDR_START_WAVERAM] = val;
        return;
    }

    switch (address) {
    case MEMADDR_NR52:
        // TODO turning off clears all APU registers and makes them readonly, except for NR52
        apu.NR52   = (WRITEABLE_NR52 & val) | (READONLY_NR52 & apu.NR52) | (USEPINS_NR52 ^ 0xFF);
        break;
    case MEMADDR_NR51:                          apu.NR51 = val; break;
    case MEMADDR_NR50:                          apu.NR50 = val; break;

    case MEMADDR_NR10:                          apu.NR10 = val | (USEPINS_NR10 ^ 0xFF); break;
    case MEMADDR_NR11:                          apu.NR11 = val; break;
    case MEMADDR_NR12:                          apu.NR12 = val; break;
    case MEMADDR_NR13:                          apu.NR13 = val; break;
    case MEMADDR_NR14:
        apu.NR14 = val | (USEPINS_NR14 ^ 0xFF);
        if (CH1_TRIGGER) trigger_ch1();
        break;

    case MEMADDR_NR21:                          apu.NR21 = val; break;
    case MEMADDR_NR22:                          apu.NR22 = val; break;
    case MEMADDR_NR23:                          apu.NR23 = val; break;
    case MEMADDR_NR24:
        apu.NR24 = val | (USEPINS_NR24 ^ 0xFF);
        if (CH2_TRIGGER) trigger_ch2();
        break;

    case MEMADDR_NR30:                          apu.NR30 = val | (USEPINS_NR30 ^ 0xFF); break;
    case MEMADDR_NR31:                          apu.NR31 = val; break;
    case MEMADDR_NR32:                          apu.NR32 = val | (USEPINS_NR32 ^ 0xFF); break;
    case MEMADDR_NR33:                          apu.NR33 = val; break;
    case MEMADDR_NR34:
        apu.NR34 = val | (USEPINS_NR34 ^ 0xFF);
        if (CH3_TRIGGER) trigger_ch3();
        break;

    case MEMADDR_NR41:                          apu.NR41 = val | (USEPINS_NR41 ^ 0xFF); break;
    case MEMADDR_NR42:                          apu.NR42 = val; break;
    case MEMADDR_NR43:                          apu.NR43 = val; break;
    case MEMADDR_NR44:
        apu.NR44 = val | (USEPINS_NR44 ^ 0xFF);
        if (CH4_TRIGGER) trigger_ch4();
        break;

    default:                                    return UNREADABLE;
    }
}
static bus_interface_t bus_registers_apu =  { .read = read_apu_reg, .write = write_apu_reg };

void
cycle_512hz_apu(void)
{
    apu.DIV_APU++;
    if (apu.DIV_APU % 2 == 0) {
        cycle_length_timers();
    }

    if (apu.DIV_APU % 8 == 7) {
        cycle_envelope_ch1();
        cycle_envelope_ch2();
        cycle_envelope_ch4();
    }

    if (apu.DIV_APU % 4 == 2) {
        cycle_sweep_ch1();
    }
}

gb_apu_t *
init_gameboy_apu(gb_bus_t *bus)
{
    apu.ch1.envelope.reg = &apu.NR12;
    apu.ch2.envelope.reg = &apu.NR22;
    apu.ch4.envelope.reg = &apu.NR42;
    bus->interface_reg_apu = &bus_registers_apu;
    return &apu;
}