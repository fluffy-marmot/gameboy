from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_serial_output

blargg_cpu_instrs = list((DATA_BLARGG / "cpu_instrs").rglob("*.gb"))

# @pytest.mark.skip("Passes reliably, a bit slow")
@pytest.mark.parametrize("testrom", blargg_cpu_instrs, ids=lambda f: f.stem)
def test_blargg_cpu_instrs(testrom: Path) -> None:
    run_blargg_test_serial_output(testrom, timeout_sec=60)
