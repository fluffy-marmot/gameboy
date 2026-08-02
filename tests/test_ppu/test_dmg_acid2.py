from tests.config import *

import pytest

from tests.test_ppu.helpers import run_image_test

def test_dmg_acid2():
    run_image_test(DATA_DMG_ACID2 / "dmg-acid2.gb")