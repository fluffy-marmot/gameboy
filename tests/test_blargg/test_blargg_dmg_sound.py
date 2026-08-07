from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_memory_signature

blargg_dmg_sound = list((DATA_BLARGG / "dmg_sound").rglob("*.gb"))

# @pytest.mark.xfail(reason="DMG audio not implemented yet") # TODO
@pytest.mark.parametrize("testrom", blargg_dmg_sound, ids=lambda f: f.stem)
def test_blargg_dmg_sound(testrom: Path) -> None:
   run_blargg_test_memory_signature(testrom)
