from pathlib import Path

from common.python.bindings import *

def GB_load_rom(romfile: Path) -> None:
    with open(romfile, "rb") as f:
        rom_data = f.read()
        buf = (uint8 * len(rom_data)).from_buffer_copy(rom_data)
        GB.GB_load_rom(buf, len(rom_data))

def GB_set_post_boot_state() -> None:
    GB.GB_set_post_boot_state()

# defaults to the colors required for image comparisons by Matt Currie tests
def GB_set_lcd_colors(clr0: int=0xFFFFFFFF, clr1:int=0xFFAAAAAA, clr2:int=0xFF555555, clr3:int=0xFF000000):
    GB.GB_set_lcd_colors((uint32 * 4)(clr0, clr1, clr2, clr3))

def GB_emulate_frame() -> None:
    GB.GB_emulate_frame()

def GB_emulate_until_opcode(opcode: int) -> None:
    GB.GB_emulate_until_opcode(opcode)

def GB_reboot_system() -> None:
    GB.GB_reboot_system()