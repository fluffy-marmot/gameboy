from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_blargg.helpers import run_blargg_test_serial_output

blargg_cgb_sound = sorted((DATA_BLARGG / "cgb_sound").rglob("*.gb"))

@pytest.mark.skip("CGB test only")
@pytest.mark.parametrize("testrom", blargg_cgb_sound, ids=lambda f: f.stem)
def test_blargg_cgb_sound(testrom: Path) -> None:
    run_blargg_test_serial_output(testrom)
