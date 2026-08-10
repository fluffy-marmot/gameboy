from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_memory_signature

blargg_oam_bug = sorted((DATA_BLARGG / "oam_bug").rglob("*.gb"))

# @pytest.mark.skip("obscure OAM bug unimplemented") # TODO
@pytest.mark.parametrize("testrom", blargg_oam_bug, ids=lambda f: f.stem)
def test_blargg_oam_bug(testrom: Path) -> None:
    run_blargg_test_memory_signature(testrom, timeout_sec=10)
