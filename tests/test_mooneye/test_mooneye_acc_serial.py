from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test

mooneye_serial = list((DATA_MOONEYE_ACCEPTANCE / "serial").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_serial, ids=lambda f: f.stem)
def test_mooneye_serial(testrom, cpu):
    run_mooneye_test(testrom, cpu)

