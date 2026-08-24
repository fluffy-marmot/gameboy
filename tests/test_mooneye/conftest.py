from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

@pytest.fixture(autouse=True)
def mooneye_test_conditions():
    GB_reboot_system()
    GB_set_post_boot_state()

@pytest.fixture(scope="package")
def cpu():
    yield gb.cpu.contents
