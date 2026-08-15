from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test, mooneye_params

mooneye_interrupts = sorted((DATA_MOONEYE_ACCEPTANCE / "interrupts").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_params(mooneye_interrupts))
def test_mooneye_interrupts(testrom, cpu):
    run_mooneye_test(testrom, cpu)

