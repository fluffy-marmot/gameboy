from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test, mooneye_params

mooneye_timer = sorted((DATA_MOONEYE_ACCEPTANCE / "timer").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_params(mooneye_timer))
def test_mooneye_timer(testrom, cpu):
    run_mooneye_test(testrom, cpu)
