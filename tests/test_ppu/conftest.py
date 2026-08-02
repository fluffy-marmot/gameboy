import pytest

from common.python.bindings import *
from common.python.common import *
from tests.config import *

@pytest.fixture(autouse=True)
def ppu_test_conditions():
    GB_reboot_system()
    GB_set_post_boot_state()
    GB_set_lcd_colors()
