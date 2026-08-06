#ifndef GB_APU
#define GB_APU

#include "_specification.h"
#include "bus.h"

typedef uint8_t digital;
typedef float analog;

typedef struct {
    analog left;
    analog right;
} stereo_sample_t;

typedef struct {
    uint8_t timer;
    uint8_t volume;
    uint8_t *reg;
} envelope_data_t;

typedef struct {
    uint8_t timer;
    uint16_t shadow_frequency;
    uint8_t enabled;
} sweep_data_t;

typedef struct {
    uint8_t timer;
    uint8_t cycle;
    uint8_t *reg;
} waveduty_data_t;

// TODO bindings
typedef struct {
    uint8_t NR52;                               // audio master control
    uint8_t NR51;                               // sound panning
    uint8_t NR50;                               // master volume and VIN panning

    uint8_t NR10;                               // channel 1 sweep
    uint8_t NR11;                               // channel 1 length timer & duty cycle
    uint8_t NR12;                               // channel 1 volume & envelope
    uint8_t NR13;                               // channel 1 period low
    uint8_t NR14;                               // channel 1 period high & control

    uint8_t NR21;                               // channel 2 length timer & duty cycle
    uint8_t NR22;                               // channel 2 volume & envelope
    uint8_t NR23;                               // channel 2 period low
    uint8_t NR24;                               // channel 2 period high & control

    uint8_t NR30;                               // channel 3 DAC enable
    uint8_t NR31;                               // channel 3 length timer
    uint8_t NR32;                               // channel 3 output level
    uint8_t NR33;                               // channel 3 period low
    uint8_t NR34;                               // channel 3 period high & control

    uint8_t NR41;                               // channel 4 length timer
    uint8_t NR42;                               // channel 4 volume & envelope
    uint8_t NR43;                               // channel 4 frequency & randomness
    uint8_t NR44;                               // channel 4 control
    
    struct {
        waveduty_data_t waveduty;
        envelope_data_t envelope;
        sweep_data_t sweep;

        uint8_t len_timer;
    } ch1;

    struct {
        waveduty_data_t waveduty;
        envelope_data_t envelope;

        uint8_t len_timer;
    } ch2;

    struct {
        uint16_t len_timer;

        uint8_t waveram[GB_DMG_WAVERAM_SIZE];       // 16 bytes of RAM for channel 3 samples
    } ch3;

    struct {
        envelope_data_t envelope;

        uint8_t len_timer;
    } ch4;

    uint8_t DIV_APU;                            // counter driving the APU

} gb_apu_t;

gb_apu_t *init_gameboy_apu(gb_bus_t *);
void cycle_512hz_apu(void);

#endif

/*
CH1-2 pulse-width modulated waves, 4 fixed pulse width settings
CH 3 - wave channel
CH 4 - pseudo random noise channel

VIN - analog from cartridge? implement?
*/