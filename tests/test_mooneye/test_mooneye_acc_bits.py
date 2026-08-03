from common.python.bindings import *
from common.python.common import *
from tests.config import *

import pytest

from tests.test_mooneye.helpers import run_mooneye_test

mooneye_bits = list((DATA_MOONEYE_ACCEPTANCE / "bits").rglob("*.gb"))

@pytest.mark.parametrize("testrom", mooneye_bits, ids=lambda f: f.stem)
def test_mooneye_bits(testrom, cpu):
    run_mooneye_test(testrom, cpu)

