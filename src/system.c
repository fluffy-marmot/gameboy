#include "system.h"

#include <stdio.h>

gb_gameboy_t gb;

__attribute__((constructor)) // gcc / clang thing
int
boot_system (void)
{
    gb.bus = init_gameboy_bus();
    gb.irq = init_gameboy_irq(gb.bus);
    gb.cpu = init_gameboy_cpu(gb.bus, gb.irq);
    gb.ppu = init_gameboy_ppu(gb.bus, gb.irq);
    gb.dma = init_gameboy_dma(gb.bus);
    gb.joypad = init_gameboy_joypad(gb.bus);
    gb.timers = init_gameboy_timers(gb.bus, gb.irq);
    
    if (init_bootrom(BOOT_ROM, gb.bus) == -1)
        printf("Error loading bootrom file: %s\n", BOOT_ROM);

    if (init_cartridge(CARTRIDGE_ROM, gb.bus) == -1)
        printf("Error loading cartridge rom file: %s\n", CARTRIDGE_ROM);
}

// TODO better abi interface and maybe cycle methods in structs? also name convention on them
void
emulate_frame(void)
{
    for (int mcycle = 0; mcycle < MACHINE_CYCLES_PER_FRAME; mcycle++) {
        cycle_mcycle_dma();
        tick_machine_cycle();
        for (int dot = 0; dot < 4; dot++) {
            cycle_tcycle_timers();
            dot_cycle();
        }
    }
}

void
set_post_boot_state(void)
{
    gb.cpu->A = 0x01;
    gb.cpu->F = 0xB0;
    gb.cpu->B = 0x00;
    gb.cpu->C = 0x13;
    gb.cpu->D = 0x00;
    gb.cpu->E = 0xD8;
    gb.cpu->H = 0x01;
    gb.cpu->L = 0x4D;
    gb.cpu->SP = 0xFFFE;
    gb.cpu->PC = 0x0100;
    gb.bus->bus_dispatcher->write(0xFF50, 1); // lock boot ROM
}