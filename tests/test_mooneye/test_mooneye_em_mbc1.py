from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test

mooneye_em_mbc1 = sorted((DATA_MOONEYE_EMULATOR_ONLY / "mbc1").rglob("*.gb"))

# @pytest.mark.skip(reason="Need to fix seg fault")
@pytest.mark.parametrize("testrom", mooneye_em_mbc1, ids=lambda f: f.stem)
def test_mooneye_em_mbc1(testrom, cpu):
    run_mooneye_test(testrom, cpu, timeout_sec=60)
