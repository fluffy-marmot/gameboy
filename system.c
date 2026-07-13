#include "system.h"

gb_gameboy_t gb;
gb_bus_t bus;

int
main(void)
{
    gb.cpu = &cpu;
    init_test_bus(&bus);
    gb.cpu->bus = &bus;
}