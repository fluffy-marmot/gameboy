#include "system.h"

#include <stdio.h>

gb_gameboy_t gb;
gb_bus_t bus;

__attribute__((constructor)) // gcc / clang thing
int
main(void)
{
    gb.cpu = &cpu;
    init_test_bus(&bus);
    gb.cpu->bus = &bus;
}