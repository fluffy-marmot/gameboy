from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_serial_output

blargg_instr_timing = sorted((DATA_BLARGG / "instr_timing").rglob("*.gb"))

@pytest.mark.parametrize("testrom", blargg_instr_timing, ids=lambda f: f.stem)
def test_blargg_instr_timing(testrom: Path) -> None:
    run_blargg_test_serial_output(testrom)
