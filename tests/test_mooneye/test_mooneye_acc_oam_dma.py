from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test, mooneye_params

mooneye_oam_dma = sorted((DATA_MOONEYE_ACCEPTANCE / "oam_dma").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_params(mooneye_oam_dma))
def test_mooneye_oam_dma(testrom, cpu):
    run_mooneye_test(testrom, cpu)

