from tests.config import *

import pytest

from tests.test_ppu.helpers import run_image_test

mealybug_roms = list(DATA_MEALYBUG.rglob("*.gb"))

# @pytest.mark.xfail(reason="torture PPU timing test will come much later")
@pytest.mark.parametrize("mealytest", mealybug_roms, ids=lambda f: f.stem)
def test_mealybug(mealytest, total_pixels):
    run_image_test(mealytest, total_pixels)