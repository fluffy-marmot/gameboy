from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test

blargg_mem_timing = list((DATA_BLARGG / "mem_timing").rglob("*.gb"))

@pytest.mark.parametrize("testrom", blargg_mem_timing, ids=lambda f: f.stem)
def test_blargg_mem_timing(testrom: Path) -> None:
    run_blargg_test(testrom)
