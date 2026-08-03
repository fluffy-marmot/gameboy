from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

blargg_roms = list(DATA_BLARGG.rglob("*.gb"))


@pytest.fixture(scope="module", autouse=True)
def blargg_test_conditions():
    GB_reboot_system()
    GB_set_post_boot_state()


@pytest.mark.parametrize("testrom", blargg_roms, ids=lambda f: f.stem)
def test_blargg(testrom: Path) -> None:
