from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

blargg_cgb_sound = list((DATA_BLARGG / "cgb_sound").rglob("*.gb"))
blargg_cpu_instrs = list((DATA_BLARGG / "cpu_instrs").rglob("*.gb"))
blargg_dmg_sound = list((DATA_BLARGG / "dmg_sound").rglob("*.gb"))
blargg_instr_timing = list((DATA_BLARGG / "instr_timing").rglob("*.gb"))
blargg_interrupt_time = list((DATA_BLARGG / "interrupt_time").rglob("*.gb"))
blargg_mem_timing = list((DATA_BLARGG / "mem_timing").rglob("*.gb"))
blargg_mem_timing2 = list((DATA_BLARGG / "mem_timing-2").rglob("*.gb"))
blargg_oam_bug = list((DATA_BLARGG / "oam_bug").rglob("*.gb"))
blargg_halt_bug = list((DATA_BLARGG).glob("*.gb"))

@pytest.fixture(autouse=True)
def blargg_test_conditions():
    GB_reboot_system()
    GB_set_post_boot_state()


def run_blargg_test(testrom: Path, timeout_sec=20) -> None:
    GB_load_rom(testrom)

    collected_output = ""

    for frame in range(timeout_sec * 60):
        GB_emulate_frame()
        new_output = GB_serial_buffer_flush()
        if new_output: 
            collected_output += new_output
        if collected_output.rstrip().endswith(("Passed", "Failed", "Passed all tests")):
            break
    else:
        pytest.fail(f"{testrom.stem}: TIMEOUT, Output: {collected_output[-40:]}", pytrace=False)

    if collected_output.rstrip().endswith("Failed"):
        pytest.fail(f"{testrom.stem}: FAILURE", pytrace=False)

@pytest.mark.skip("CGB test only")
@pytest.mark.parametrize("testrom", blargg_cgb_sound, ids=lambda f: f.stem)
def test_blargg_cgb_sound(testrom: Path) -> None:
    run_blargg_test(testrom)

@pytest.mark.skip("Passes reliably, a bit slow")
@pytest.mark.parametrize("testrom", blargg_cpu_instrs, ids=lambda f: f.stem)
def test_blargg_cpu_instrs(testrom: Path) -> None:
    run_blargg_test(testrom, timeout_sec=60)

@pytest.mark.skip("DMG audio not implemented yet") # TODO
@pytest.mark.parametrize("testrom", blargg_dmg_sound, ids=lambda f: f.stem)
def test_blargg_dmg_sound(testrom: Path) -> None:
    run_blargg_test(testrom)

@pytest.mark.parametrize("testrom", blargg_instr_timing, ids=lambda f: f.stem)
def test_blargg_instr_timing(testrom: Path) -> None:
    run_blargg_test(testrom)

@pytest.mark.skip("interrupt time seems to depend on some basic APU") # TODO
@pytest.mark.parametrize("testrom", blargg_interrupt_time, ids=lambda f: f.stem)
def test_blargg_interrupt_time(testrom: Path) -> None:
    run_blargg_test(testrom)

@pytest.mark.parametrize("testrom", blargg_mem_timing, ids=lambda f: f.stem)
def test_blargg_mem_timing(testrom: Path) -> None:
    run_blargg_test(testrom)

@pytest.mark.skip("Pass but don't write to serial port")
@pytest.mark.parametrize("testrom", blargg_mem_timing2, ids=lambda f: f.stem)
def test_blargg_mem_timing2(testrom: Path) -> None:
    run_blargg_test(testrom)

@pytest.mark.skip("obscure OAM bug unimplemented") # TODO
@pytest.mark.parametrize("testrom", blargg_oam_bug, ids=lambda f: f.stem)
def test_blargg_oam_bug(testrom: Path) -> None:
    run_blargg_test(testrom, timeout_sec=60)

@pytest.mark.parametrize("testrom", blargg_halt_bug, ids=lambda f: f.stem)
def test_blargg_halt_bug(testrom: Path) -> None:
    run_blargg_test(testrom)
