from common.python.bindings import *
from common.python.common import *
from tests.config import *

import fnmatch
import pytest

SKIP_ROMS = {
    "*-dmg0": "DMG0 model only",
    "*-mgb":  "MGB model only",
    "*-sgb":  "SGB model only",
    "*-S":    "SGB model only",
    "*-sgb2": "SGB2 model only",
}

def mooneye_params(testroms):
    params = []
    for rom in testroms:
        reason = next(
            (why for pattern, why in SKIP_ROMS.items() if fnmatch.fnmatchcase(rom.stem, pattern)), None
        )
        marks = [pytest.mark.skip(reason=reason)] if reason else []
        params.append(pytest.param(rom, id=rom.stem, marks=marks))
    return params

def run_mooneye_test(testrom: Path, cpu) -> None:
    GB_load_rom(testrom)
    result = GB_emulate_until_opcode(0x40, max_mcycles=20_000_000)

    if result != 0:
        pytest.fail(f"{testrom.stem}: timeout before LD B B breakpoint", pytrace=False)
    elif (cpu.B, cpu.C, cpu.D, cpu.E, cpu.H, cpu.L) == (3, 5, 8, 13, 21, 34):
        pass
    elif (cpu.B, cpu.C, cpu.D, cpu.E, cpu.H, cpu.L) == (0x42, 0x42, 0x42, 0x42, 0x42, 0x42):
        pytest.fail(f"{testrom.stem}: test reported fail state", pytrace=False)
    else:
        pytest.fail(f"{testrom.stem}: unknown register status ", pytrace=False)