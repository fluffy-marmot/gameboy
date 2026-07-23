#include "system.h"

#include <stdio.h>

gb_gameboy_t gb;

__attribute__((constructor)) // gcc / clang thing
int
boot_system (void)
{
    gb.bus = init_gameboy_bus();
    gb.cpu = init_gameboy_cpu(gb.bus);
    
    if (init_bootrom(BOOT_ROM, gb.bus) == -1)
        printf("Error loading bootrom file: %s\n", BOOT_ROM);
    
    // on init - map boot ROM to 0x0000 - 0x00FF
}