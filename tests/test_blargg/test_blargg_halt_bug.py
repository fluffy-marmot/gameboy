from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_ppu.helpers import run_image_test

blargg_halt_bug = sorted((DATA_BLARGG).glob("*.gb"))

# this test doesn't seem to output to either serial or external WRAM signature (cartridge header
# reports no external RAM), so automating by comparison to a reference image like ppu tests
@pytest.mark.parametrize("testrom", blargg_halt_bug, ids=lambda f: f.stem)
def test_blargg_halt_bug(testrom: Path) -> None:
    GB_set_lcd_colors()
    run_image_test(testrom, None)
