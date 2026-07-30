#ifndef GB_ABIHDR
#define GB_ABIHDR

#include "_specification.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GB_ABI __attribute__((visibility("default")))

typedef enum {
    GB_RETURN_OK,
    GB_ERROR,
    GB_ERROR_MALLOC,
    GB_ERROR_BOOTROM_SIZE,
    GB_ERROR_ROM_SIZE_MINIMUM,
} gb_return_t;

// bootrom.c
// load bootrom data (abi client handles file reading)
GB_ABI gb_return_t GB_load_bootrom(const uint8_t *data, size_t size);

// bus.c
// alternate memory mode and read / write functions for using a flat memory layout during some tests
GB_ABI gb_return_t GB_test_memory_mode_enable(void);
GB_ABI gb_return_t GB_test_memory_mode_disable(void);
GB_ABI gb_return_t GB_test_memory_wipe(void);
GB_ABI        void GB_test_memory_write(memaddr address, uint8_t value);
GB_ABI     uint8_t GB_test_memory_read (memaddr address);

// cartridge.c
// load ROM program cartridge data (abi client handles file reading)
GB_ABI gb_return_t GB_load_rom(const uint8_t *data, size_t size);

// emulator.c
GB_ABI gb_return_t GB_set_post_boot_state(void);
GB_ABI gb_return_t GB_emulate_frame(void);

// joypad.c
// allow client to update states of joypad controller's 8 buttons (false means pressed button)
GB_ABI gb_return_t GB_update_joypad(bool start, bool select, bool b,    bool a, 
                                    bool down,  bool up,     bool left, bool right);

// ppu.c
// get display data for LCD screen
GB_ABI uint32_t *GB_get_lcd(void);

#endif