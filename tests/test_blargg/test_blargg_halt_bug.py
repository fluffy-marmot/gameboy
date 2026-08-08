from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_serial_output

blargg_halt_bug = sorted((DATA_BLARGG).glob("*.gb"))

@pytest.mark.parametrize("testrom", blargg_halt_bug, ids=lambda f: f.stem)
def test_blargg_halt_bug(testrom: Path) -> None:
    run_blargg_test_serial_output(testrom)
