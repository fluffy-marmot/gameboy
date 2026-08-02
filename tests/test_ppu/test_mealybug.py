from tests.config import *

import pytest

from tests.test_ppu.helpers import run_image_test

@pytest.mark.xfail(reason="torture PPU timing test will come much later")
@pytest.mark.parametrize("mealytest", list(DATA_MEALYBUG.rglob("*.gb")), ids=lambda f: f.stem)
def test_mealybug(mealytest):
    run_image_test(mealytest)