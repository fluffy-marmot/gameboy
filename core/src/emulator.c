#include "_abi.h"
#include "emulator.h"

gb_gameboy_t gb;

// returns true if a vblank occured during this period, using this in
// emulate_frame to return early to help resync display after LCD off / on
static bool
emulate_machine_cycle(void)
{
    bool encountered_vblank = false;
    cycle_mcycle_dma();
    cycle_mcycle_cpu();
    for (int tcycle = 0; tcycle < DOTS_PER_MACHINE_CYCLE; tcycle++) {
        cycle_tcycle_timers();
        if (gb.timers->serial_falling_edge)
            cycle_serial();
        if (gb.timers->apu_falling_edge)
            cycle_512hz_apu();
        encountered_vblank |= cycle_tcycle_ppu();
    }
    return encountered_vblank;
}

/* ############################################################################
###############################################################################

        client-facing ABI functions
        
###############################################################################
############################################################################ */

__attribute__((constructor))
gb_return_t
GB_reboot_system(void)
{
    gb.bus = init_gameboy_bus();
    gb.irq = init_gameboy_irq(gb.bus);
    gb.cpu = init_gameboy_cpu(gb.bus, gb.irq);
    gb.ppu = init_gameboy_ppu(gb.bus, gb.irq);
    gb.dma = init_gameboy_dma(gb.bus);
    gb.apu = init_gameboy_apu(gb.bus);
    gb.joypad = init_gameboy_joypad(gb.bus, gb.irq);
    gb.serial = init_gameboy_serial(gb.bus, gb.irq);
    gb.timers = init_gameboy_timers(gb.bus, gb.irq);
    gb.cartridge = init_cartridge(gb.bus);
    init_bootrom(gb.bus);

    return GB_RETURN_OK;
}

// reach post-boot state without needing to load a boot ROM
gb_return_t
GB_set_post_boot_state(void)
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
    gb.bus->bus_dispatcher->write(MEMADDR_LCDC, 0x91);              // LCDC - enable 
    gb.bus->bus_dispatcher->write(MEMADDR_BGP, 0xFC);               // BGP - set palette as bootrom
    gb.bus->bus_dispatcher->write(MEMADDR_BOOT_ROM_LOCK, 1);        // lock boot ROM

    return GB_RETURN_OK;
}

// turn all the gears and such
gb_return_t
GB_emulate_frame(void)
{
    for (int mcycle = 0; mcycle < MACHINE_CYCLES_PER_FRAME; mcycle++) {
        // emulate a machine cycle and check whether a vblank occurred
        // in which case, this should return early to resync w/ PPU
        // (otherwise LCD when drawn may contain data from two different frames)
        if (emulate_machine_cycle() && mcycle != MACHINE_CYCLES_PER_FRAME - 1) {
            // printf("Early resync return after %d machine cycles\n", mcycle);
            return GB_RETURN_RESYNC_VBLANK;
        }
    }
    return GB_RETURN_OK;
}

gb_return_t
GB_test_single_instruction(int max_mcycles)
{
    int i = 0;
    do {
        cycle_mcycle_cpu();
        i++;
    } while (i < max_mcycles &&
             (gb.cpu->cb_instruction || gb.cpu->cycle_num != gb.cpu->instruction->cycle_count));
    return GB_RETURN_OK;
}

// useful for mealybug tests which check generated LCD image using LD B, B opcode as breakpoint
gb_return_t
GB_emulate_until_opcode(uint8_t opcode, uint32_t max_mcycles)
{
    while (gb.cpu->instruction == NULL || gb.cpu->cb_instruction || gb.cpu->IR != opcode) {
        emulate_machine_cycle();
        if (!--max_mcycles)
            return GB_RETURN_OPCODE_BREAKPOINT_TIMEOUT;
    }
    return GB_RETURN_OK;
}