#ifndef GB_APU
#define GB_APU

#include "_specification.h"
#include "bus.h"

typedef struct {
    uint8_t NR52;                               // audio master control


} gb_apu_t;

gb_apu_t *init_gameboy_apu(gb_bus_t *);

#endif

/*
CH1-2 pulse-width modulated waves, 4 fixed pulse width settings
CH 3 - wave channel
CH 4 - pseudo random noise channel

VIN - analog from cartridge? implement?
*/