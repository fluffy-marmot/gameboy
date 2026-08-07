from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_serial_output

blargg_interrupt_time = list((DATA_BLARGG / "interrupt_time").rglob("*.gb"))

# @pytest.mark.skip("interrupt time seems to depend on some basic APU") # TODO
@pytest.mark.parametrize("testrom", blargg_interrupt_time, ids=lambda f: f.stem)
def test_blargg_interrupt_time(testrom: Path) -> None:
    run_blargg_test_serial_output(testrom)
