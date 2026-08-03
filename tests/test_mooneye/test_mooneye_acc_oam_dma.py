from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test

mooneye_oam_dma = list((DATA_MOONEYE_ACCEPTANCE / "oam_dma").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_oam_dma, ids=lambda f: f.stem)
def test_mooneye_oam_dma(testrom, cpu):
    run_mooneye_test(testrom, cpu)

