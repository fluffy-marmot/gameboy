import ctypes
from pathlib import Path

BASE_DIR = Path(__file__).parent.parent.parent.resolve()
GB = ctypes.CDLL(BASE_DIR / "build" / "gameboy.so")

class CPU(ctypes.Structure):
    B: int # can add type annotations? TODO
    _fields_ = [
        ("B", ctypes.c_uint8),
        ("C", ctypes.c_uint8),
        ("D", ctypes.c_uint8),
        ("E", ctypes.c_uint8),
        ("H", ctypes.c_uint8),
        ("L", ctypes.c_uint8),
        ("F", ctypes.c_uint8),
        ("A", ctypes.c_uint8),
        ("PC", ctypes.c_uint16),
        ("SP", ctypes.c_uint16),
        ("IR", ctypes.c_uint8),
        ("_pad0", ctypes.c_uint8),  # oops thats due to not defining the union python side, alignment issues
        ("Z", ctypes.c_uint8),
        ("W", ctypes.c_uint8),
        ("IME", ctypes.c_uint8),
        ("IME_latch", ctypes.c_uint8),
        ("bus", ctypes.c_void_p),
        ("irq", ctypes.c_void_p),
        ("cycle_num", ctypes.c_uint8),
        ("cb_instruction", ctypes.c_uint8),
        ("instruction", ctypes.c_void_p),
    ]
    
    def load_test(self, test):
        for key in test:
            if key == "ram": continue
            self.__setattr__(key.upper(), test[key])
        self.IR = 0
        self.Z = 0
        self.W = 0
        self.IME_latch = 0

    def test_final(self, test):
        ok = True
        for key in test:
            if key in ["ram", "ei"]: continue
            if getattr(self, key.upper()) != test[key]:
                print(f"\t\t{key}, actual: {getattr(self, key.upper()):08b}, expected: {test[key]:08b}")
                ok = False
        if not ok:
            print()
        return ok


# bus.h

GB.test_memory_mode_enable.restype = None
GB.test_memory_mode_enable.argtypes = []

GB.test_memory_mode_disable.restype = None
GB.test_memory_mode_disable.argtypes = []

GB.test_memory_wipe.restype = None
GB.test_memory_wipe.argtypes = []

GB.test_memory_write.restype = None
GB.test_memory_write.argtypes = [ctypes.c_uint16, ctypes.c_uint8]

GB.test_memory_read.restype = ctypes.c_uint8
GB.test_memory_read.argtypes = [ctypes.c_uint16]

# cpu.h

cpu = CPU.in_dll(GB, "cpu")

# GB.fetch_instruction.restype = None
# GB.fetch_instruction.argtypes = []

GB.tick_machine_cycle.restype = ctypes.c_uint16
GB.tick_machine_cycle.argtypes = []

# ppu.h

GB.dot_cycle.restype = None
GB.dot_cycle.argtypes = []

GB.emulate_frame.restype = None
GB.emulate_frame.argtypes = []

GB.get_lcd.argtypes = []
GB.get_lcd.restype = ctypes.POINTER(ctypes.c_uint32)
LCD = GB.get_lcd()
# lcd_type = ctypes.c_uint32 * (160 * 144)
# LCD = ctypes.cast(ptr, ctypes(lcd_type)).contents

GB.set_post_boot_state.argtypes = []
GB.set_post_boot_state.restype = None

GB.update_joypad.argtypes = [ctypes.c_bool, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool, ctypes.c_bool]
GB.update_joypad.restype = None