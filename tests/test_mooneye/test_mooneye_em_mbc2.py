from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test

mooneye_em_mbc2 = list((DATA_MOONEYE_EMULATOR_ONLY / "mbc2").rglob("*.gb"))

@pytest.mark.skip(reason="Unimplemented")
@pytest.mark.parametrize("testrom", mooneye_em_mbc2, ids=lambda f: f.stem)
def test_mooneye_em_mbc2(testrom, cpu):
    run_mooneye_test(testrom, cpu)
