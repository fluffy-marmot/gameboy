import ctypes as ct
from pathlib import Path

BASE_DIR = Path(__file__).parent.parent.parent.resolve()
GB = ct.CDLL(BASE_DIR / "build" / "gameboy.so")

MAX_MCYCLE_INSTRUCTION = 6

GB_DMG_VRAM_SIZE = 8 * 1024
GB_DMG_HRAM_SIZE = 127
GB_DMG_OAM_SIZE = 160
SCANLINE_MAX_OBJS = 10
LCD_WIDTH = 160
LCD_HEIGHT = 144

c_bool = ct.c_bool
c_int = ct.c_int
uint8 = ct.c_uint8
uint16 = ct.c_uint16
uint32 = ct.c_uint32
pointer = ct.POINTER
void_p = ct.c_void_p
struct = ct.Structure
function = ct.CFUNCTYPE
size_t = ct.c_size_t

memaddr = uint16

bus_read_func = function(uint8, memaddr)
bus_write_func = function(None, memaddr, uint8)

cycle_func = function(None)

check_next_func = function(c_int)
call_interrupt_func = function(memaddr, c_int)
update_stat_func = function(None, uint8)

class bus_interface_t(struct):
    _fields_ = [
        ("read", bus_read_func),
        ("write", bus_write_func),
    ]

class cart_data_t(struct):
    _fields_ = [
        ("rom", pointer(uint8)),
        ("ram", pointer(uint8)),
        ("rom_size", size_t),
        ("ram_size", size_t),
    ]

class cartridge_header(struct):
    _fields_ = [
        ("mbc_type", c_int),
        ("rom_size_id", c_int),
        ("ram_size_id", c_int),
    ]

class instruction(struct):
    _fields_ = [
        ("cycles", cycle_func * MAX_MCYCLE_INSTRUCTION),
        ("cycle_count", uint8),
    ]

class gb_irq_handler_t(struct):
    _fields_ = [
        ("IE", uint8),
        ("IF", uint8),
        ("check_next", check_next_func),
        ("call_interrupt", call_interrupt_func),
        ("update_stat", update_stat_func),
    ]

class gb_dma_t(struct):
    _fields_ = [
        ("DMA", uint8),

        ("cycles_active", uint8),
        ("data", uint8),
        ("status", c_int),

        ("bus", pointer(bus_interface_t)),
    ]

class gb_bus_t(struct):
    _fields_ = [
        ("bus_dispatcher", pointer(bus_interface_t)),

        ("interface_rom_boot", pointer(bus_interface_t)),
        ("interface_cartridge", pointer(bus_interface_t)),
        ("interface_vram", pointer(bus_interface_t)),
        ("interface_wram_system", pointer(bus_interface_t)),
        ("interface_echo", pointer(bus_interface_t)),
        ("interface_oam", pointer(bus_interface_t)),
        ("interface_unusable", pointer(bus_interface_t)),
        ("interface_hram", pointer(bus_interface_t)),

        ("interface_reg_bootlock", pointer(bus_interface_t)),
        ("interface_reg_ppu", pointer(bus_interface_t)),
        ("interface_reg_interrupt", pointer(bus_interface_t)),
        ("interface_reg_timers", pointer(bus_interface_t)),
        ("interface_reg_joypad", pointer(bus_interface_t)),

        ("interface_dma", pointer(bus_interface_t)),

        ("interface_nop", pointer(bus_interface_t)),

        ("BOOT_ROM_LOCK", uint8),
        ("dma", pointer(gb_dma_t)),
    ]

class gb_cartridge_t(struct):
    _fields_ = [
        ("header", cartridge_header),

        ("data", cart_data_t),
        ("mbc_bus", pointer(gb_bus_t)),
    ]

class gb_cpu_t(struct):
    B: int
    C: int
    D: int
    E: int
    H: int
    L: int
    F: int
    A: int

    PC: int
    SP: int
    IR: int

    Z: int
    W: int

    IME: int
    IME_latch: int

    cycle_num: int
    cb_instruction: int

    _fields_ = [
        ("B", uint8),
        ("C", uint8),
        ("D", uint8),
        ("E", uint8),
        ("H", uint8),
        ("L", uint8),
        ("F", uint8),
        ("A", uint8),

        ("PC", uint16),
        ("SP", uint16),
        ("IR", uint8),

        ("Z", uint8),
        ("W", uint8),

        ("IME", uint8),
        ("IME_latch", uint8),

        ("bus", pointer(bus_interface_t)),
        ("irq", pointer(gb_irq_handler_t)),

        ("cycle_num", uint8),
        ("cb_instruction", uint8),
        ("instruction", pointer(instruction)),
    ]

class gb_joypad_t(struct):
    _fields_ = [
        ("JOYP", uint8),

        ("BUTTON_SELECT", c_bool),
        ("BUTTON_START", c_bool),
        ("BUTTON_B", c_bool),
        ("BUTTON_A", c_bool),
        ("BUTTON_DOWN", c_bool),
        ("BUTTON_UP", c_bool),
        ("BUTTON_LEFT", c_bool),
        ("BUTTON_RIGHT", c_bool),

        ("bus", pointer(bus_interface_t)),
        ("irq", pointer(gb_irq_handler_t)),
    ]

class gb_ppu_t(struct):
    _fields_ = [
        ("LCDC", uint8),
        ("STAT", uint8),
        ("SCY", uint8),
        ("SCX", uint8),
        ("LY", uint8),
        ("LYC", uint8),

        ("BGP", uint8),
        ("OBP", uint8 * 2),
        ("WY", uint8),
        ("WX", uint8),

        ("vram", uint8 * GB_DMG_VRAM_SIZE),
        ("oam", uint8 * GB_DMG_OAM_SIZE),
        ("lcd", uint32 * (LCD_HEIGHT * LCD_WIDTH)),

        ("lx", uint8),
        ("line_dot", c_int),
        ("obj_buffer", uint8 * SCANLINE_MAX_OBJS),
        ("obj_buffer_size", uint8),

        ("irq", pointer(gb_irq_handler_t)),
    ]

class gb_timers_t(struct):
    _fields_ = [
        ("DIV", uint16),
        ("TIMA", uint8),
        ("TMA", uint8),
        ("TAC", uint8),

        ("timer_operation_stored", uint8),
        ("tima_overflow_cycles", uint8),

        ("irq", pointer(gb_irq_handler_t)),
    ]

class gb_gameboy_t(struct):
    _fields_ = [
        ("cpu", pointer(gb_cpu_t)),
        ("bus", pointer(gb_bus_t)),
        ("dma", pointer(gb_dma_t)),
        ("ppu", pointer(gb_ppu_t)),
        ("joypad", pointer(gb_joypad_t)),
        ("timers", pointer(gb_timers_t)),
        ("irq", pointer(gb_irq_handler_t)),
        ("cartridge", pointer(gb_cartridge_t)),
    ]

GB.GB_test_memory_mode_enable.restype = None
GB.GB_test_memory_mode_enable.argtypes = []

GB.GB_test_memory_mode_disable.restype = None
GB.GB_test_memory_mode_disable.argtypes = []

GB.GB_test_memory_wipe.restype = None
GB.GB_test_memory_wipe.argtypes = []

GB.GB_test_memory_write.restype = None
GB.GB_test_memory_write.argtypes = [uint16, uint8]

GB.GB_test_memory_read.restype = uint8
GB.GB_test_memory_read.argtypes = [uint16]

GB.GB_emulate_frame.restype = None
GB.GB_emulate_frame.argtypes = []

GB.GB_test_single_instruction.restype = None
GB.GB_test_single_instruction.argtypes = []

GB.GB_get_lcd.argtypes = []
GB.GB_get_lcd.restype = ct.POINTER(uint32)

GB.GB_set_post_boot_state.argtypes = []
GB.GB_set_post_boot_state.restype = None

GB.GB_update_joypad.argtypes = [
    ct.c_bool, ct.c_bool, ct.c_bool, ct.c_bool, ct.c_bool, ct.c_bool, ct.c_bool, ct.c_bool]
GB.GB_update_joypad.restype = None

GB.GB_load_bootrom.argtypes = [ct.POINTER(uint8), ct.c_size_t]
GB.GB_load_bootrom.restype = ct.c_int

GB.GB_load_rom.argtypes = [ct.POINTER(uint8), ct.c_size_t]
GB.GB_load_rom.restype = ct.c_int

LCD = ct.cast(GB.GB_get_lcd(), ct.POINTER(uint32 * (LCD_WIDTH * LCD_HEIGHT))).contents
gb = gb_gameboy_t.in_dll(GB, "gb")