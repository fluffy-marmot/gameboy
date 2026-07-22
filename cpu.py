import ctypes
import json
from pathlib import Path

BASE_DIR = Path(__file__).parent.resolve()

gb = ctypes.CDLL("./gameboy.so")

class CPU(ctypes.Structure):
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
        ("IME", ctypes.c_uint8),
        ("IME_latch", ctypes.c_uint8),
        ("Z", ctypes.c_uint8),
        ("W", ctypes.c_uint8),
        ("bus", ctypes.c_void_p),
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
                print(f"\t\t{key}, actual: {getattr(self, key.upper()):>3d}, expected: {test[key]:>3d}")
                ok = False
        if not ok:
            print()
        return ok
                
gb.main()

cpu = CPU.in_dll(gb, "cpu")

gb.memory_read.restype = ctypes.c_uint8
gb.memory_read.argtypes = [ctypes.c_uint16]

gb.memory_write.restype = None
gb.memory_write.argtypes = [ctypes.c_uint16, ctypes.c_uint8]

gb.memory_wipe.restype = None
gb.memory_wipe.argtypes = []

SM83_DIR = BASE_DIR / "sm83" / "v1"

non_cb_tests = sorted(f for f in SM83_DIR.glob("*.json") if not f.name.startswith("cb "))
cb_tests = sorted(f for f in SM83_DIR.glob("cb *.json"))

def test_ram(ram):
    ok = True
    for addr, val in ram:
        if gb.memory_read(addr) != val:
            print(f"\t\tAddress {addr:#X}, actual: {gb.memory_read(addr):>3d}, expected: {val:>3d}")
            ok = False
    if not ok:
        print()
    return ok

results = dict()
perfect = 0
for test_file in non_cb_tests:
    correct, wrong = 0, 0
    with open(test_file) as f:
        tests = json.load(f)
    for i, test in enumerate(tests):
        cpu.load_test(test["initial"])
        gb.memory_wipe()
        for addr, val in test["initial"]["ram"]:
            gb.memory_write(addr, val)
        gb.instruction_test()

        if not cpu.test_final(test["final"]) or not test_ram(test["final"]["ram"]):
            print(f"\t↑↑↑ Test # {i}")
            # print(test)
            wrong += 1
        else: correct += 1
    print(f"{test_file.name}: {correct} / 1000")
    if correct == 1000:
        perfect += 1
    else:
        print(f"↑↑↑ {test["name"]}")
        # input()
    # input()
    results[test_file.stem] = correct

print({key: val for key, val in results.items() if val != 1000})
passed_total = sum(results.values())
total = len(non_cb_tests) * 1000
print(f"Total tests: {passed_total} / {total}, {passed_total / total * 100: .2f} %, Instructions passing: {perfect} / {len(non_cb_tests)}")


    # print(tests[0])
    # print()
    # print(tests[0]["initial"])
    # print()


# if __name__ == "__main__":
#     main()