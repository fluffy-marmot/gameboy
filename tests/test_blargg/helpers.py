from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

def run_blargg_test_serial_output(testrom: Path, timeout_sec=20) -> None:
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


def run_blargg_test_memory_signature(testrom: Path, timeout_sec=20) -> None:
    """
    From Blargg's readme.txt: 
    Output to memory
    ----------------
    Text output and the final result are also written to memory at $A000,
    allowing testing a very minimal emulator that supports little more than
    CPU and RAM. To reliably indicate that the data is from a test and not
    random data, $A001-$A003 are written with a signature: $DE,$B0,$61. If
    this is present, then the text string and final result status are valid.

    $A000 holds the overall status. If the test is still running, it holds
    $80, otherwise it holds the final result code.
    """
    GB_load_rom(testrom)

    for frame in range(timeout_sec * 60):
        GB_emulate_frame()

    external_ram = ct.cast(gb.cartridge.contents.data.ram, pointer(uint8))
    # Blargg signature
    if (external_ram[1] == 0xDE and external_ram[2] == 0xB0 and external_ram[3] == 0x61):
        if (external_ram[0] != 0):
            pytest.fail(f"{testrom.stem}: FAILURE, result status: {external_ram[0]}", pytrace=False)
    else:
        pytest.fail(f"{testrom.stem}: FAILURE, invalid memory signature", pytrace=False)

