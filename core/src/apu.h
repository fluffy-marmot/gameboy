#ifndef GB_APU
#define GB_APU

#include "_specification.h"
#include "bus.h"

#define AUDIO_SAMPLE_RATE                       44100   // Hz
#define AUDIO_BUFFER_CAPACITY                   1024    // samples

typedef uint8_t digital;
typedef float analog;

typedef struct {
    analog left;
    analog right;
} stereo_sample_t;

typedef struct {
    uint16_t size;
    stereo_sample_t data[AUDIO_BUFFER_CAPACITY];
} audio_buffer_t;

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
    uint16_t timer;
    uint8_t cycle;
    uint8_t *reg;
} waveduty_data_t;
// Important not to move first 5 members of the struct: besides buffer, 
/* when APU is turned off, waveram, DIV_APU aren't zeroed out, along w/ length timer bits */ 
typedef struct {
    audio_buffer_t buf;
    int sample_timer;

    uint8_t waveram[GB_DMG_WAVERAM_SIZE];       // 16 bytes of RAM for channel 3 samples
    uint8_t DIV_APU;                            // counter driving the APU
    
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
        uint8_t len_timer;
        envelope_data_t envelope;
        waveduty_data_t waveduty;
        sweep_data_t sweep;
    } ch1;
    struct {
        uint8_t len_timer;
        envelope_data_t envelope;
        waveduty_data_t waveduty;
    } ch2;
    struct {
        uint16_t len_timer;
        uint16_t wave_timer;
        uint8_t sample_index;
    } ch3;
    struct {
        uint8_t len_timer;
        envelope_data_t envelope;
        uint8_t sample_index;
    } ch4;

    uint8_t waveram_transaction;
} gb_apu_t;

gb_apu_t *init_gameboy_apu(gb_bus_t *);
void cycle_512hz_apu_frame_sequencer(void);
void cycle_mcycle_apu_pulse_channels(void);
void cycle_2tcycles_apu_wave_channel(void);
void cycle_tcycle_apu_emit_sample(void);

#endif

// VIN implementation? seems unlikely...