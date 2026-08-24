#ifndef GB_RTC
#define GB_RTC

#include <time.h>

#include "../cartridge.h"

#define RTC_SELECT_ACCESS_LOW                   0x4000
#define RTC_SELECT_ACCESS_HIGH                  0x5FFF

#define RTC_LATCH_ACCESS_LOW                    0x6000
#define RTC_LATCH_ACCESS_HIGH                   0x7FFF

#define RTC_REGISTER_ACCESS_LOW                 0xA000
#define RTC_REGISTER_ACCESS_HIGH                0xBFFF

typedef struct {
    uint8_t RTC_S;
    uint8_t RTC_M;
    uint8_t RTC_H;
    uint8_t RTC_DL;
    uint8_t RTC_DH;
} rtc_clock_t;

typedef struct {
    rtc_clock_t clock;
    rtc_clock_t clock_latched;

    uint8_t selected;
    uint8_t latch;
    uint8_t oscillator_step;
    uint16_t second_timer;

    bus_interface_t *bus_rtc;
    time_t timestamp;
} gb_rtc_t;

gb_rtc_t *init_gameboy_rtc(void);
void cycle_tcycle_rtc(void);

#endif
