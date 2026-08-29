#include "rtc.h"

#define RTC_CLOCK_FREQUENCY                     32768

#define USEPINS_RTC_S                           0b00111111
#define USEPINS_RTC_M                           0b00111111
#define USEPINS_RTC_H                           0b00011111
#define USEPINS_RTC_DH                          0b11000001

#define RTC_DH_BIT_DAY_COUNTER_HIGH_BIT         0b00000001
#define RTC_DH_BIT_CARRY_BIT                    0b10000000
#define RTC_DH_BIT_TIMER_HALT                   0b01000000

#define RTC_DH_DAY_COUNTER_HIGH_BIT             (rtc.clock.RTC_DH & RTC_DH_BIT_DAY_COUNTER_HIGH_BIT)
#define RTC_DH_TIMER_HALT                       (rtc.clock.RTC_DH & RTC_DH_BIT_TIMER_HALT)

#define RTC_SELECTED_S                          0x08
#define RTC_SELECTED_M                          0x09
#define RTC_SELECTED_H                          0x0A
#define RTC_SELECTED_DL                         0x0B
#define RTC_SELECTED_DH                         0x0C

#define RTC_LATCH_INITIAL                       0b00000001

static const uint8_t TCYCLES_PER_RTC_TICK     = DOTS_PER_SECOND / RTC_CLOCK_FREQUENCY;

gb_rtc_t rtc;

static uint8_t
read_bus_rtc(memaddr address)
{
    if (ADDR_RANGE(RTC_REGISTER_ACCESS_LOW, RTC_REGISTER_ACCESS_HIGH)) {
        switch (rtc.selected) {
        case RTC_SELECTED_S:                        return rtc.clock_latched.RTC_S;
        case RTC_SELECTED_M:                        return rtc.clock_latched.RTC_M;
        case RTC_SELECTED_H:                        return rtc.clock_latched.RTC_H;
        case RTC_SELECTED_DL:                       return rtc.clock_latched.RTC_DL;
        case RTC_SELECTED_DH:                       return rtc.clock_latched.RTC_DH;
        }
    }
    return UNREADABLE;
}
static void
write_bus_rtc(memaddr address, uint8_t val)
{
    if (ADDR_RANGE(RTC_SELECT_ACCESS_LOW, RTC_SELECT_ACCESS_HIGH)) {
        rtc.selected = val;
    } else if (ADDR_RANGE(RTC_LATCH_ACCESS_LOW, RTC_LATCH_ACCESS_HIGH)) {
        if (val == 0x00) {
            rtc.latch = 0x00;
        } else if (val == 0x01 && rtc.latch == 0x00) {
            rtc.latch = 0x01;
            memcpy(&rtc.clock_latched, &rtc.clock, sizeof(rtc_clock_t));
        }
    } else if (ADDR_RANGE(RTC_REGISTER_ACCESS_LOW, RTC_REGISTER_ACCESS_HIGH)) {
        // TODO: seems like this shouldn't also affect the latched value, Ashiepaws guide is wrong?
        switch (rtc.selected) {
        case RTC_SELECTED_S:
            rtc.clock_latched.RTC_S = rtc.clock.RTC_S = val & USEPINS_RTC_S;
            rtc.second_timer = 0;
            // rtc.oscillator_step = 0;
            break;
        case RTC_SELECTED_M:
            rtc.clock_latched.RTC_M = rtc.clock.RTC_M = val & USEPINS_RTC_M;
            break;
        case RTC_SELECTED_H:
            rtc.clock_latched.RTC_H = rtc.clock.RTC_H = val & USEPINS_RTC_H;
            break;
        case RTC_SELECTED_DL:
            rtc.clock_latched.RTC_DL = rtc.clock.RTC_DL = val;
            break;
        case RTC_SELECTED_DH:
            rtc.clock_latched.RTC_DH = rtc.clock.RTC_DH = val & USEPINS_RTC_DH;
            break;
        }
    }
}
static bus_interface_t bus_rtc = { .read = read_bus_rtc, .write = write_bus_rtc };

void
cycle_tcycle_rtc(void)
{
    if (++rtc.oscillator_step != TCYCLES_PER_RTC_TICK) return;
    rtc.oscillator_step = 0;

    if (RTC_DH_TIMER_HALT) return;

    if (++rtc.second_timer != RTC_CLOCK_FREQUENCY) return;
    rtc.second_timer = 0;

    rtc.clock.RTC_S = (rtc.clock.RTC_S + 1) & USEPINS_RTC_S;
    if (rtc.clock.RTC_S == 60) {
        rtc.clock.RTC_S = 0;
        rtc.clock.RTC_M = (rtc.clock.RTC_M + 1) & USEPINS_RTC_M;
        if (rtc.clock.RTC_M == 60) {
            rtc.clock.RTC_M = 0;
            rtc.clock.RTC_H = (rtc.clock.RTC_H + 1) & USEPINS_RTC_H;
            if (rtc.clock.RTC_H == 24) {
                rtc.clock.RTC_H = 0;
                if (++rtc.clock.RTC_DL == 0) {
                    rtc.clock.RTC_DH ^= RTC_DH_BIT_DAY_COUNTER_HIGH_BIT;
                    if (!RTC_DH_DAY_COUNTER_HIGH_BIT)
                        rtc.clock.RTC_DH |= RTC_DH_BIT_CARRY_BIT;
                }
            }
        }
    }
}

gb_rtc_t *
init_gameboy_rtc(void)
{
    memset(&rtc, 0, sizeof(gb_rtc_t));
    memset(&rtc.clock_latched, 0xFF, sizeof(rtc_clock_t));
    rtc.latch = RTC_LATCH_INITIAL;
    rtc.bus_rtc = &bus_rtc;
    return &rtc;
}
