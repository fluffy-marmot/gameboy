from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

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
