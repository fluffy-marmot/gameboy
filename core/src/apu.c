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

#define CH1_PAN_LEFT                            (apu.NR51 & NR51_BIT_CH1_LEFT)
#define CH2_PAN_LEFT                            (apu.NR51 & NR51_BIT_CH2_LEFT)
#define CH3_PAN_LEFT                            (apu.NR51 & NR51_BIT_CH3_LEFT)
#define CH4_PAN_LEFT                            (apu.NR51 & NR51_BIT_CH4_LEFT)
#define CH1_PAN_RIGHT                           (apu.NR51 & NR51_BIT_CH1_RIGHT)
#define CH2_PAN_RIGHT                           (apu.NR51 & NR51_BIT_CH2_RIGHT)
#define CH3_PAN_RIGHT                           (apu.NR51 & NR51_BIT_CH3_RIGHT)
#define CH4_PAN_RIGHT                           (apu.NR51 & NR51_BIT_CH4_RIGHT)

// NR50 - master volume & VIN panning
#define NR50_BIT_VIN_LEFT                       0b10000000
#define NR50_BITS_LEFT_VOLUME                   0b01110000
#define NR50_BIT_VIN_RIGHT                      0b00001000
#define NR50_BITS_RIGHT_VOLUME                  0b00000111

#define LEFT_VOLUME                             ((apu.NR50 & NR50_BITS_LEFT_VOLUME) >> 4)
#define RIGHT_VOLUME                            (apu.NR50 & NR50_BITS_RIGHT_VOLUME)

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
#define CH1_SWEEP_DIR_DOWN                      (apu.NR10 & NR10_BIT_SWEEP_DIR)
#define CH1_SWEEP_SHIFT                         (apu.NR10 & NR10_BITS_SWEEP_SHIFT)

// NR11 - channel 1 length timer & duty cycle
#define READABLE_NR11                           0b11000000

#define NR11_BITS_WAVE_DUTY                     0b11000000
#define NR11_BITS_INIT_LENGTH_TIMER             0b00111111

#define CH1_WAVE_DUTY                           ((apu.NR11 & NR11_BITS_WAVE_DUTY) >> 6)
#define CH1_INIT_LENGTH_TIMER                   (apu.NR11 & NR11_BITS_INIT_LENGTH_TIMER)

#define CH1_DAC_ENABLED                         (apu.NR12 & BITMASK_ENVELOPE_DAC_ENABLED)

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

#define CH2_DAC_ENABLED                         (apu.NR22 & BITMASK_ENVELOPE_DAC_ENABLED)

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

#define CH3_DAC_ENABLED                         (apu.NR30 & NR30_BIT_DAC)

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

#define CH3_PERIOD                              (((apu.NR34 & NR34_BITS_PERIOD_HIGH) << 8) | apu.NR33)
#define CH3_LENGTH_TIMER_ENABLED                (apu.NR34 & NR34_BIT_LENGTH_ENABLE)
#define CH3_TRIGGER                             (apu.NR34 & NR34_BIT_TRIGGER)

//////////////////////////////////////////////////////////
/* CHANNEL 4 */
//////////////////////////////////////////////////////////

// NR41 - channel 4 length timer
#define USEPINS_NR41                            0b00111111

#define NR41_BITS_INIT_LENGTH_TIMER             0b00111111

#define CH4_INIT_LENGTH_TIMER                   (apu.NR41 & NR41_BITS_INIT_LENGTH_TIMER)

#define CH4_DAC_ENABLED                         (apu.NR42 & BITMASK_ENVELOPE_DAC_ENABLED)

// NR43 - channel 4 frequency & randomness
#define NR43_BITS_CLOCK_SHIFT                   0b11110000
#define NR43_BIT_LFSR_WIDTH                     0b00001000
#define NR43_BITS_CLOCK_DIVIDER                 0b00000111

#define CH4_CLOCK_SHIFT                         ((apu.NR43 & NR43_BITS_CLOCK_SHIFT) >> 4)
#define CH4_LFSR_WIDTH                          (apu.NR43 & NR43_BIT_LFSR_WIDTH)
#define CH4_CLOCK_DIVISOR_CODE                  (apu.NR43 & NR43_BITS_CLOCK_DIVIDER)

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
#define PERIOD_COUNTER_OVERFLOW                 2048

#define MASK_BIT_L                              0b00000001
#define MASK_NIBBLE_L                           0b00001111
#define READABLE_NRx4                           0b01000000

// Envelope stuff for ch 1, 2, 4
#define BITMASK_ENVELOPE_DAC_ENABLED            0b11111000
#define BITMASK_ENVELOPE_INITIAL_VOLUME         0b11110000
#define BITMASK_ENVELOPE_DIR                    0b00001000
#define BITMASK_ENVELOPE_PACE                   0b00000111

#define ZOMBIE_MODE_NIBBLE_VAL                  0b00001000

#define ENVELOPE_INITIAL_VOLUME(env_reg)        ((*(env_reg) & BITMASK_ENVELOPE_INITIAL_VOLUME) >> 4)
#define ENVELOPE_DIR_UP(env_reg)                (*(env_reg) & BITMASK_ENVELOPE_DIR)
#define ENVELOPE_PACE(env_reg)                  (*(env_reg) & BITMASK_ENVELOPE_PACE)

static const uint8_t DUTY_WAVEFORM[4] = {
    [0b00] = 0b00000001,                        // 12.5 % ratio
    [0b01] = 0b00000011,                        // 25.0 % ratio
    [0b10] = 0b00001111,                        // 50.0 % ratio
    [0b11] = 0b11111100                         // 75.0 % ratio
};

static const uint8_t CH3_VOLUME_SHIFT[4] = {
    [0b00] = 0b00000100,
    [0b01] = 0b00000000,
    [0b10] = 0b00000001,
    [0b11] = 0b00000010,
};

static const uint8_t CH4_DIVISOR[8] = {
    [0b000] =  8,
    [0b001] = 16,
    [0b010] = 32,
    [0b011] = 48,
    [0b100] = 64,
    [0b101] = 80,
    [0b110] = 96,
    [0b111] = 112
};

static const analog DAC[16] = {
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

// TODO Examine more: For CH1 only: when the period sweep overflows4, or
// The channel’s DAC is turned off. The envelope reaching a volume of 0 does NOT turn the channel off!
static void
tick_envelope(envelope_data_t *env)
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
    new_frequency = apu.ch1.sweep.shadow_frequency + new_frequency * (CH1_SWEEP_DIR_DOWN ? -1 : 1);
    if (CH1_SWEEP_DIR_DOWN)
        apu.ch1.sweep.subtraction_since_trigger = true;
    if (new_frequency > CH1_MAX_FREQUENCY)
        DISABLE_CH(1);
    return new_frequency;
}

static void
tick_sweep_ch1(void)
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

static void tick_length_timer1(void) {
    if (apu.DIV_APU % 2 != 0) return;
    if (CH1_LENGTH_TIMER_ENABLED && apu.ch1.len_timer && (--apu.ch1.len_timer == 0))    DISABLE_CH(1);
}

static void tick_length_timer2(void) {
    if (apu.DIV_APU % 2 != 0) return;
    if (CH2_LENGTH_TIMER_ENABLED && apu.ch2.len_timer && (--apu.ch2.len_timer == 0))    DISABLE_CH(2);
}

static void tick_length_timer3(void) {
    if (apu.DIV_APU % 2 != 0) return;
    if (CH3_LENGTH_TIMER_ENABLED && apu.ch3.len_timer && (--apu.ch3.len_timer == 0))    DISABLE_CH(3);
}

static void tick_length_timer4(void) {
    if (apu.DIV_APU % 2 != 0) return;
    if (CH4_LENGTH_TIMER_ENABLED && apu.ch4.len_timer && (--apu.ch4.len_timer == 0))    DISABLE_CH(4);
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
    apu.ch1.waveduty.timer = CH1_PERIOD;

    // length timer
    if (apu.ch1.len_timer == 0) {
        apu.ch1.len_timer = SHORT_TIMER;
        // Due to quirk tested by Blargg's 03-trigger test, if clock enable bit and DIV-APU % 0 
        tick_length_timer1();
    }

    // envelope
    trigger_envelope(&apu.ch1.envelope);
    // enable before sweep overflow check, which could keep it disabled
    if (CH1_DAC_ENABLED)
        ENABLE_CH(1);
    // sweep
    apu.ch1.sweep.subtraction_since_trigger = false;
    apu.ch1.sweep.shadow_frequency = CH1_PERIOD;
    apu.ch1.sweep.timer = CH1_SWEEP_PACE ? CH1_SWEEP_PACE : 8;
    apu.ch1.sweep.enabled = CH1_SWEEP_PACE | CH1_SWEEP_SHIFT;
    if (CH1_SWEEP_SHIFT)
        calculate_sweep_frequency_and_check_overflow_ch1();
}

static void
trigger_ch2(void)
{
    apu.ch2.waveduty.timer = CH2_PERIOD;

    // length timer
    if (apu.ch2.len_timer == 0) {
        apu.ch2.len_timer = SHORT_TIMER;
        // Due to quirk tested by Blargg's 03-trigger test, if clock enable bit and DIV-APU % 0 
        tick_length_timer2();
    }

    // envelope
    trigger_envelope(&apu.ch2.envelope);

    // enable
    if (CH2_DAC_ENABLED)
        ENABLE_CH(2);
}

static void
trigger_ch3(void)
{
    apu.ch3.wave_timer = CH3_PERIOD * 2;
    apu.ch3.sample_index = 0;

    // long length timer
    if (apu.ch3.len_timer == 0) {
        apu.ch3.len_timer = LONG_TIMER;
        // Due to quirk tested by Blargg's 03-trigger test, if clock enable bit and DIV-APU % 0 
        tick_length_timer3();
    }

    // enable
    if (CH3_DAC_ENABLED)
        ENABLE_CH(3);
}

static void
trigger_ch4(void)
{
    // length timer
    if (apu.ch4.len_timer == 0) {
        apu.ch4.len_timer = SHORT_TIMER;
        // Due to quirk tested by Blargg's 03-trigger test, if clock enable bit and DIV-APU % 0 
        tick_length_timer4();
    }
    // TODO ?? "except that shift being equal to 14 or 15 stops the channel from being clocked entirely."
    apu.ch4.noise.timer = CH4_DIVISOR[CH4_CLOCK_DIVISOR_CODE] << CH4_CLOCK_SHIFT;
    apu.ch4.noise.LFSR = 0x7FFF;

    // envelope
    trigger_envelope(&apu.ch4.envelope);

    // enable
    if (CH4_DAC_ENABLED)
        ENABLE_CH(4);
}

static analog
dac_ch1(void)
{
    digital ch1_dig = 0x0;
    if (CH1_ON) {
        uint8_t ch1_vol = apu.ch1.envelope.volume;
        ch1_dig = ((DUTY_WAVEFORM[CH1_WAVE_DUTY] >> apu.ch1.waveduty.cycle) & MASK_BIT_L) * ch1_vol;
    }
    return DAC[ch1_dig & 0xF];
}

static analog
dac_ch2(void)
{
    digital ch2_dig = 0x0;
    if (CH2_ON) {
        uint8_t ch2_vol = apu.ch2.envelope.volume;
        ch2_dig = ((DUTY_WAVEFORM[CH2_WAVE_DUTY] >> apu.ch2.waveduty.cycle) & MASK_BIT_L) * ch2_vol;
    }
    return DAC[ch2_dig & 0xF];
}

static analog
dac_ch3(void)
{
    if (!CH3_ON) return DAC[0x0];

    uint8_t sample_byte = apu.waveram[apu.ch3.sample_index / 2];
    digital sample = ((apu.ch3.sample_index % 2 == 0) ? (sample_byte >> 4) : (sample_byte)) & MASK_NIBBLE_L;
    sample >>= CH3_VOLUME_SHIFT[CH3_OUTPUT_LEVEL];
    return DAC[sample & 0xF];
}

static analog
dac_ch4(void)
{
    if (!CH4_ON) return DAC[0x0];

    digital ch4_dig = ((~apu.ch4.noise.LFSR) & MASK_BIT_L) * apu.ch4.envelope.volume;
    return DAC[ch4_dig & 0xF];
}

static stereo_sample_t
mix_sample(void)
{
    analog signal;
    stereo_sample_t sample = {0};

    if (CH1_DAC_ENABLED) {
        signal = dac_ch1();
        if (CH1_PAN_LEFT)   sample.left  += signal;
        if (CH1_PAN_RIGHT)  sample.right += signal;
    }
    if (CH2_DAC_ENABLED) {
        signal = dac_ch2();
        if (CH2_PAN_LEFT)   sample.left  += signal;
        if (CH2_PAN_RIGHT)  sample.right += signal;
    }
    if (CH3_DAC_ENABLED) {
        signal = dac_ch3();
        if (CH3_PAN_LEFT)   sample.left  += signal;
        if (CH3_PAN_RIGHT)  sample.right += signal;
    }
    if (CH4_DAC_ENABLED) {
        signal = dac_ch4();
        if (CH4_PAN_LEFT)   sample.left  += signal;
        if (CH4_PAN_RIGHT)  sample.right += signal;
    }
    sample.left /= 4.0f;
    sample.right /= 4.0f;
    return sample;
}

static stereo_sample_t
volume_scale_sample(stereo_sample_t sample)
{
    sample.left *= (LEFT_VOLUME + 1) / 8.0f;
    sample.right *= (RIGHT_VOLUME + 1) / 8.0f;
    return sample;
}

static stereo_sample_t
high_pass_filter_sample(stereo_sample_t sample)
{
    static double capacitor_left = 0.0;
    static double capacitor_right = 0.0;

    stereo_sample_t output_sample = { .left = 0.0f, .right = 0.0f };
    if (CH1_DAC_ENABLED || CH2_DAC_ENABLED || CH3_DAC_ENABLED) {
        output_sample.left = sample.left - capacitor_left;
        output_sample.right = sample.right - capacitor_right;

        capacitor_left = sample.left - output_sample.left * 0.996013;
        capacitor_right = sample.right - output_sample.right * 0.996013;
    }
    return output_sample;
}

static void
apu_power_off(void)
{
    uint8_t ch1_init_len_timer = CH1_INIT_LENGTH_TIMER;
    uint8_t ch2_init_len_timer = CH2_INIT_LENGTH_TIMER;
    uint8_t ch3_init_len_timer = CH3_INIT_LENGTH_TIMER;
    uint8_t ch4_init_len_timer = CH4_INIT_LENGTH_TIMER;
    uint8_t ch1_len_timer = apu.ch1.len_timer;
    uint8_t ch2_len_timer = apu.ch2.len_timer;
    uint16_t ch3_len_timer = apu.ch3.len_timer;
    uint8_t ch4_len_timer = apu.ch4.len_timer;

    // zero out everything except waveram, DIV_APU, and NR52
    memset(&apu.NR52, 0x00, sizeof(gb_apu_t) - offsetof(gb_apu_t, NR52));

    // memset above wipes these back-pointers (they live inside ch1/ch2/ch4); restore them
    apu.ch1.envelope.reg = &apu.NR12;
    apu.ch2.envelope.reg = &apu.NR22;
    apu.ch4.envelope.reg = &apu.NR42;

    // the length timer bits also don't get zeroed out
    apu.NR11 = ch1_init_len_timer;
    apu.NR21 = ch2_init_len_timer;
    apu.NR31 = ch3_init_len_timer;
    apu.NR41 = ch4_init_len_timer;
    apu.ch1.len_timer = ch1_len_timer;
    apu.ch2.len_timer = ch2_len_timer;
    apu.ch3.len_timer = ch3_len_timer;
    apu.ch4.len_timer = ch4_len_timer;
}

static uint8_t 
read_apu_reg (memaddr address)
{
    // Haven't figured out a proper way to handle waveram bus conflicts
    if (ADDR_RANGE(ADDR_START_WAVERAM, ADDR_END_WAVERAM)) {
        if (!CH3_ON)                            return apu.waveram[address - ADDR_START_WAVERAM];
        else                                    return UNREADABLE;
    }

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
    if (address == MEMADDR_NR52) {
        uint8_t apu_enabled_before = APU_ENABLED;
        apu.NR52 = (WRITEABLE_NR52 & val) | (READONLY_NR52 & apu.NR52) | (USEPINS_NR52 ^ 0xFF);
        if (apu_enabled_before && !APU_ENABLED)
            apu_power_off();
        else if (!apu_enabled_before && APU_ENABLED)
            // Blargg tests dmg-sound-07 and 08 (maybe?), need next frame sequencer cycle to run with "0 cycle"
            // so we need DIV_APU to be 0 after it gets incremented
            apu.DIV_APU = 0xFF;
        return;
    } else if (ADDR_RANGE(ADDR_START_WAVERAM, ADDR_END_WAVERAM)) {
        if (!CH3_ON)
            apu.waveram[address - ADDR_START_WAVERAM] = val;
        return;
    } else if (!APU_ENABLED) {
        switch (address) {
            case MEMADDR_NR11:
                apu.NR11 = val & NR11_BITS_INIT_LENGTH_TIMER;
                apu.ch1.len_timer = SHORT_TIMER - CH1_INIT_LENGTH_TIMER;
                return;
            case MEMADDR_NR21:
                apu.NR21 = val & NR21_BITS_INIT_LENGTH_TIMER;
                apu.ch2.len_timer = SHORT_TIMER - CH2_INIT_LENGTH_TIMER;
                return;
            case MEMADDR_NR31:
                apu.NR31 = val;
                apu.ch3.len_timer = LONG_TIMER - CH3_INIT_LENGTH_TIMER;
                return;
            case MEMADDR_NR41:
                apu.NR41 = val & NR41_BITS_INIT_LENGTH_TIMER;
                apu.ch4.len_timer = SHORT_TIMER - CH4_INIT_LENGTH_TIMER;
                return;
            default:
                return;
        }
    }

    switch (address) {
    case MEMADDR_NR51:                          apu.NR51 = val; break;
    case MEMADDR_NR50:                          apu.NR50 = val; break;

    case MEMADDR_NR10:
        apu.NR10 = val | (USEPINS_NR10 ^ 0xFF);
        /* Pandocs: "Clearing the sweep direction bit in NR10 after at least one sweep calculation has been made
        using the substraction mode since the last trigger causes the channel to be immediately disabled." */
        if (apu.ch1.sweep.subtraction_since_trigger && !CH1_SWEEP_DIR_DOWN)
            DISABLE_CH(1);
        break;
    case MEMADDR_NR11:
        apu.NR11 = val;
        apu.ch1.len_timer = SHORT_TIMER - CH1_INIT_LENGTH_TIMER;
        break;
    case MEMADDR_NR12:                          
        apu.NR12 = val;
        if (!CH1_DAC_ENABLED) DISABLE_CH(1);
        else if (CH1_ON && (val & MASK_NIBBLE_L) == ZOMBIE_MODE_NIBBLE_VAL)
            // zombie mode quirk that increments volume
            apu.ch1.envelope.volume = (apu.ch1.envelope.volume + 1) % 0x10;
        break;
    case MEMADDR_NR13:                          apu.NR13 = val; break;
    case MEMADDR_NR14:
        uint8_t ch1_before_len_timer_enabled = CH1_LENGTH_TIMER_ENABLED;
        apu.NR14 = val | (USEPINS_NR14 ^ 0xFF);
        // Due to quirk tested by Blargg's 03-trigger test - rising edge causes a tick of len timer
        if (!ch1_before_len_timer_enabled && CH1_LENGTH_TIMER_ENABLED)
            tick_length_timer1();

        if (CH1_TRIGGER)
            trigger_ch1();
        break;

    case MEMADDR_NR21:
        apu.NR21 = val;
        apu.ch2.len_timer = SHORT_TIMER - CH2_INIT_LENGTH_TIMER;
        break;
    case MEMADDR_NR22:
        apu.NR22 = val;
        if (!CH2_DAC_ENABLED) DISABLE_CH(2);
        else if (CH2_ON && (val & MASK_NIBBLE_L) == ZOMBIE_MODE_NIBBLE_VAL)
            // zombie mode quirk that increments volume
            apu.ch2.envelope.volume = (apu.ch2.envelope.volume + 1) % 0x10;
        break;
    case MEMADDR_NR23:                          apu.NR23 = val; break;
    case MEMADDR_NR24:
        uint8_t ch2_before_len_timer_enabled = CH2_LENGTH_TIMER_ENABLED;
        apu.NR24 = val | (USEPINS_NR24 ^ 0xFF);
        // Due to quirk tested by Blargg's 03-trigger test - rising edge causes a tick of len timer
        if (!ch2_before_len_timer_enabled && CH2_LENGTH_TIMER_ENABLED)
            tick_length_timer2();
        if (CH2_TRIGGER) trigger_ch2();
        break;

    case MEMADDR_NR30:
        apu.NR30 = val | (USEPINS_NR30 ^ 0xFF);
        if (!CH3_DAC_ENABLED) DISABLE_CH(3);
        break;
    case MEMADDR_NR31: 
        apu.NR31 = val;
        apu.ch3.len_timer = LONG_TIMER - val;
        break;
    case MEMADDR_NR32:                          apu.NR32 = val | (USEPINS_NR32 ^ 0xFF); break;
    case MEMADDR_NR33:                          apu.NR33 = val; break;
    case MEMADDR_NR34:
        uint8_t ch3_before_len_timer_enabled = CH3_LENGTH_TIMER_ENABLED;
        apu.NR34 = val | (USEPINS_NR34 ^ 0xFF);
        // Due to quirk tested by Blargg's 03-trigger test - rising edge causes a tick of len timer
        if (!ch3_before_len_timer_enabled && CH3_LENGTH_TIMER_ENABLED)
            tick_length_timer3();
        if (CH3_TRIGGER) trigger_ch3();
        break;
    case MEMADDR_NR41:
        apu.NR41 = val | (USEPINS_NR41 ^ 0xFF);
        apu.ch4.len_timer = SHORT_TIMER - CH4_INIT_LENGTH_TIMER;
        break;
    case MEMADDR_NR42:
        apu.NR42 = val;
        if (!CH4_DAC_ENABLED) DISABLE_CH(4);
        else if (CH4_ON && (val & MASK_NIBBLE_L) == ZOMBIE_MODE_NIBBLE_VAL)
            // zombie mode quirk that increments volume
            apu.ch4.envelope.volume = (apu.ch4.envelope.volume + 1) % 0x10;
        break;
    case MEMADDR_NR43:                          apu.NR43 = val; break;
    case MEMADDR_NR44:
        uint8_t ch4_before_len_timer_enabled = CH4_LENGTH_TIMER_ENABLED;
        apu.NR44 = val | (USEPINS_NR44 ^ 0xFF);
        // Due to quirk tested by Blargg's 03-trigger test - rising edge causes a tick of len timer
        if (!ch4_before_len_timer_enabled && CH4_LENGTH_TIMER_ENABLED)
            tick_length_timer4();
        if (CH4_TRIGGER) trigger_ch4();
        break;
    }
}
static bus_interface_t bus_registers_apu =  { .read = read_apu_reg, .write = write_apu_reg };

// emit samples only at rate of AUDIO_SAMPLE_RATE
void 
cycle_tcycle_apu_emit_sample(void) {
    apu.sample_timer += AUDIO_SAMPLE_RATE;
    if (apu.sample_timer > DOTS_PER_SECOND) {
        apu.sample_timer %= DOTS_PER_SECOND;
        // if we've reached max buffer size, assume client doesn't read audio buffer and discard
        if (apu.buf.size == AUDIO_BUFFER_CAPACITY)
            apu.buf.size = 0;
        apu.buf.data[apu.buf.size++] = high_pass_filter_sample(volume_scale_sample(mix_sample()));
    }
}

void
cycle_512hz_apu_frame_sequencer(void)
{
    if (!APU_ENABLED) return;
    apu.DIV_APU++;

    if (apu.DIV_APU % 4 == 2)
        tick_sweep_ch1();
    if (apu.DIV_APU % 8 == 7) {
        tick_envelope(&apu.ch1.envelope);
        tick_envelope(&apu.ch2.envelope);
        tick_envelope(&apu.ch4.envelope);
    }

    // DIV_APU % 2 check is baked into these functions, since they are also used for quirk behavior
    tick_length_timer1();
    tick_length_timer2();
    tick_length_timer3();
    tick_length_timer4();
}

void
cycle_mcycle_apu_pulse_channels(void)
{
    if (CH1_ON && ++apu.ch1.waveduty.timer == PERIOD_COUNTER_OVERFLOW) {
        apu.ch1.waveduty.timer = CH1_PERIOD;
        apu.ch1.waveduty.cycle = (apu.ch1.waveduty.cycle + 1) % 8;
    }
    if (CH2_ON && ++apu.ch2.waveduty.timer == PERIOD_COUNTER_OVERFLOW) {
        apu.ch2.waveduty.timer = CH2_PERIOD;
        apu.ch2.waveduty.cycle = (apu.ch2.waveduty.cycle + 1) % 8;
    }
}

void
cycle_tcycle_apu_wave_channel(void)
{
    if (CH3_ON && ++apu.ch3.wave_timer == PERIOD_COUNTER_OVERFLOW * 2) {
        apu.ch3.wave_timer = CH3_PERIOD * 2;
        apu.ch3.sample_index = (apu.ch3.sample_index + 1) % 32;
    }
}

void
cycle_tcycle_apu_noise_channel(void)
{
    if (CH4_ON && --apu.ch4.noise.timer == 0) {
        apu.ch4.noise.timer = CH4_DIVISOR[CH4_CLOCK_DIVISOR_CODE] << CH4_CLOCK_SHIFT;
        uint8_t low_bits_xor = (apu.ch4.noise.LFSR ^ (apu.ch4.noise.LFSR >> 1)) & MASK_BIT_L;
        apu.ch4.noise.LFSR >>= 1;
        apu.ch4.noise.LFSR |= (low_bits_xor << 14);
        if (CH4_LFSR_WIDTH) {
            apu.ch4.noise.LFSR &= ~(MASK_BIT_L << 6);
            apu.ch4.noise.LFSR |= (low_bits_xor << 6);
        }
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

/* ############################################################################
###############################################################################

        client-facing ABI functions
        
###############################################################################
############################################################################ */

// caller should read size of buf before flushing it, as it will be set to 0
float *
GB_audio_buffer_flush(void)
{
    apu.buf.size = 0;
    return (float *) apu.buf.data;
}

uint16_t
GB_audio_buffer_size(void)
{
    return apu.buf.size;
}