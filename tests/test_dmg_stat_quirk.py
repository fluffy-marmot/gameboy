from pathlib import Path

import pytest

from common.python.bindings import *
from common.python.common import *
from tests.config import *

from tests.test_blargg.helpers import run_blargg_test_memory_signature

stat_quirk_roms = sorted(DATA_STAT_QUIRK.rglob("*.gb"))

@pytest.fixture(scope="module", autouse=True)
def test_conditions():
    GB_reboot_system()
    GB_set_post_boot_state()


@pytest.mark.parametrize("testrom", stat_quirk_roms, ids=lambda f: f.stem)
def test_dmg_stat_quirk(testrom: Path) -> None:
    run_blargg_test_memory_signature(testrom, timeout_sec=1)
