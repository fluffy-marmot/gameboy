import pytest

from common.python.bindings import *
from common.python.common import *
from tests.config import *

@pytest.fixture(autouse=True)
def ppu_test_conditions():
    GB_reboot_system()
    GB_set_post_boot_state()
    GB_set_lcd_colors()

@pytest.fixture(scope="module")
def total_pixels():
    tracker = {"total": 0}
    yield tracker
    print(f"TOTAL PIXEL DIFF for mealybug tests: {tracker['total']}")
