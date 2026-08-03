from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

def run_mooneye_test(testrom: Path, cpu, timeout_sec=20) -> None:
    GB_load_rom(testrom)
    result = GB_emulate_until_opcode(0x40, max_mcycles=4_000_000)

    if result != 0:
        pytest.fail(f"{testrom.stem}: timeout before LD B B breakpoint", pytrace=False)
    elif (cpu.B, cpu.C, cpu.D, cpu.E, cpu.H, cpu.L) == (3, 5, 8, 13, 21, 34):
        pass
    elif (cpu.B, cpu.C, cpu.D, cpu.E, cpu.H, cpu.L) == (0x42, 0x42, 0x42, 0x42, 0x42, 0x42):
        pytest.fail(f"{testrom.stem}: test reported fail state", pytrace=False)
    else:
        pytest.fail(f"{testrom.stem}: unknown register status ", pytrace=False)