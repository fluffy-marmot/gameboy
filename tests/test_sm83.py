import json
from pathlib import Path

import pytest

from bindings.python.gb_c_definitions import GB, gb
from tests.config import *

sm83_jsons = sorted(DATA_SM83_DIR.glob("*.json"))
registers_8bit = "a b c d e f h l".split()


@pytest.fixture(scope="module", autouse=True)
def sm83_test_memory_mode():
    GB.GB_test_memory_mode_enable()
    yield
    GB.GB_test_memory_mode_disable()


def load_sm83_cpu_state(state: dict) -> None:
    cpu = gb.cpu.contents
    for reg in registers_8bit:
        setattr(cpu, reg.upper(), state[reg])
    cpu.PC = state["pc"]
    cpu.SP = state["sp"]
    cpu.IME = state["ime"]
    cpu.IME_latch = 0
    cpu.IR = 0
    cpu.Z = 0
    cpu.W = 0

    # wipe CPU state from previous instruction so it will execute a fetch, otherwise halt goes brrr
    cpu.instruction = None
    cpu.cb_instruction = 0

    GB.GB_test_memory_wipe()
    for addr, val in state["ram"]:
        GB.GB_test_memory_write(addr, val)


def check_sm83_cpu_state(state: dict) -> list[str]:
    cpu = gb.cpu.contents
    mismatches = []

    for reg in registers_8bit:
        actual = getattr(cpu, reg.upper())
        expected = state[reg]
        if actual != expected:
            mismatches.append(f"{reg.upper()}: actual={actual:#04x} expected={expected:#04x}")

    for field in ("pc", "sp"):
        actual = getattr(cpu, field.upper())
        expected = state[field]
        if actual != expected:
            mismatches.append(f"{field.upper()}: actual={actual:#06x} expected={expected:#06x}")

    if cpu.IME != state["ime"]:
        mismatches.append(f"IME: actual={cpu.IME} expected={state['ime']}")

    for addr, val in state["ram"]:
        actual = GB.GB_test_memory_read(addr)
        if actual != val:
            mismatches.append(f"RAM[{addr:#06x}]: actual={actual:#04x} expected={val:#04x}")

    return mismatches


@pytest.mark.parametrize("sm83_instruction_json", sm83_jsons, ids=lambda f: f.stem)
def test_instruction(sm83_instruction_json: Path) -> None:
    with open(sm83_instruction_json) as f:
        steptests = json.load(f)

    failures = []
    for i, steptest in enumerate(steptests):
        load_sm83_cpu_state(steptest["initial"])
        GB.GB_test_single_instruction(len(steptest["cycles"]))
        mismatches = check_sm83_cpu_state(steptest["final"])
        if mismatches:
            failures.append(f"steptest {i} ({steptest['name']}):\n    " + "\n    ".join(mismatches))

    if failures:
        shown = failures[:20]
        summary = f"{len(failures)}/{len(steptests)} steptests failed"
        if len(failures) > len(shown):
            summary += f" (showing first {len(shown)})"
        pytest.fail(summary + "\n" + "\n".join(shown), pytrace=False)