from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

@pytest.fixture(autouse=True)
def blargg_test_conditions():
    GB_reboot_system()
    GB_set_post_boot_state()
