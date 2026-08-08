from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test

mooneye_instr = sorted((DATA_MOONEYE_ACCEPTANCE / "instr").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_instr, ids=lambda f: f.stem)
def test_mooneye_instr(testrom, cpu):
    run_mooneye_test(testrom, cpu)

