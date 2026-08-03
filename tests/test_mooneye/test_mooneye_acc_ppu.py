from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test

mooneye_ppu = list((DATA_MOONEYE_ACCEPTANCE / "ppu").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_ppu, ids=lambda f: f.stem)
def test_mooneye_ppu(testrom, cpu):
    run_mooneye_test(testrom, cpu)

