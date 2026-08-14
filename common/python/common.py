from pathlib import Path

from PIL import Image

from common.python.bindings import *

def GB_load_rom(romfile: Path) -> None:
    with open(romfile, "rb") as f:
        rom_data = f.read()
        buf = (uint8 * len(rom_data)).from_buffer_copy(rom_data)
        GB.GB_load_rom(buf, len(rom_data))

def GB_uses_bbram() -> bool:
    return GB.GB_uses_bbram()

def GB_uses_rtc() -> bool:
    return GB.GB_uses_rtc()

def GB_load_bbram(bbram_file: Path) -> None:
    with open(bbram_file, "rb") as f:
        bbram_data = f.read()
        buf = (uint8 * len(bbram_data)).from_buffer_copy(bbram_data)
        GB.GB_load_bbram(buf, len(bbram_data))

def gb_save_bbram(bbram_file: Path) -> None:
    bbram_data = ct.string_at(gb.cartridge.contents.data.ram, get_cartridge_ram_size())
    with open(bbram_file, "wb") as f:
        f.write(bbram_data)

def GB_set_post_boot_state() -> None:
    GB.GB_set_post_boot_state()

# defaults to the colors required for image comparisons by Matt Currie tests
def GB_set_lcd_colors(clr0: int=0xFFFFFFFF, clr1:int=0xFFAAAAAA, clr2:int=0xFF555555, clr3:int=0xFF000000):
    GB.GB_set_lcd_colors((uint32 * 4)(clr0, clr1, clr2, clr3))

def GB_emulate_frame() -> None:
    GB.GB_emulate_frame()

def GB_emulate_until_opcode(opcode: int, max_mcycles=0) -> None:
    return GB.GB_emulate_until_opcode(opcode, max_mcycles)

def GB_reboot_system() -> None:
    GB.GB_reboot_system()

def GB_serial_buffer_flush() -> str | None:
    buffer_size = gb.serial.contents.buf.size
    if not buffer_size:
        return None
    raw = ct.string_at(GB.GB_serial_buffer_flush(), buffer_size)
    return raw.decode('ascii', errors='backslashreplace')

def GB_audio_buffer_flush() -> list[float] | None:
    buffer_size = gb.apu.contents.buf.size
    if not buffer_size:
        return None
    return GB.GB_audio_buffer_flush()[:buffer_size * 2]

def check_blank_frame() -> bool:
    return gb.ppu.contents.lcd.blank_frames > 0

def save_lcd_png(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.frombuffer("RGBA", (LCD_WIDTH, LCD_HEIGHT), LCD, "raw", "BGRA", 0, 1).save(path)

def get_cartridge_ram_size() -> int:
    return gb.cartridge.contents.data.ram_size

def get_cartridge_info() -> str:
    header = gb.cartridge.contents.header
    return f"Header: {header.mbc_type:02X} {header.rom_size_id:02X} {header.ram_size_id:02X} BBRAM: {GB_uses_bbram()} RTC: {GB_uses_rtc()}"