#ifndef GB_BUS
#define GB_BUS

#include "_specification.h"

#define ADDR_BELOW(high)                        (address <= high)
#define ADDR_RANGE(low, high)                   ((low <= address) && (address <= high))

typedef struct {
    uint8_t (*read )(memaddr         );
    void    (*write)(memaddr, uint8_t);
} bus_interface_t;

typedef struct {
    bus_interface_t *bus_dispatcher;

    bus_interface_t *interface_rom_boot;
    bus_interface_t *interface_cartridge;
    bus_interface_t *interface_vram;
    bus_interface_t *interface_wram_system;
    bus_interface_t *interface_echo;
    bus_interface_t *interface_oam;
    bus_interface_t *interface_unusable;
    bus_interface_t *interface_hram;

    bus_interface_t *interface_reg_bootlock;
    bus_interface_t *interface_reg_ppu;
    bus_interface_t *interface_reg_apu;
    bus_interface_t *interface_reg_interrupt;
    bus_interface_t *interface_reg_timers;
    bus_interface_t *interface_reg_joypad;
    bus_interface_t *interface_reg_serial;
    
    bus_interface_t *interface_dma;
    
    bus_interface_t *interface_oam_corruption;
    bus_interface_t *interface_nop;

    uint8_t BOOT_ROM_LOCK;

    struct gb_dma *dma;
} gb_bus_t;

gb_bus_t *init_gameboy_bus(void);

#endif