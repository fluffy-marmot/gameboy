from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_memory_signature

blargg_mem_timing2 = list((DATA_BLARGG / "mem_timing-2").rglob("*.gb"))

@pytest.mark.parametrize("testrom", blargg_mem_timing2, ids=lambda f: f.stem)
def test_blargg_mem_timing2(testrom: Path) -> None:
    run_blargg_test_memory_signature(testrom, timeout_sec=3)
