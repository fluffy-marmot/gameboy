from pathlib import Path

TESTS_DIR = Path(__file__).parent.resolve()

IMG_TEST_OUTPUT = TESTS_DIR / "output"

DATA_SM83_DIR = TESTS_DIR / "testdata" / "sm83" / "v1"
DATA_DMG_ACID2 = TESTS_DIR / "testdata" / "dmg-acid2"
DATA_MEALYBUG = TESTS_DIR / "testdata" / "mealybug"
DATA_BLARGG = TESTS_DIR / "testdata" / "blargg"
DATA_MOONEYE_ACCEPTANCE = TESTS_DIR / "testdata" / "mooneye" / "acceptance"
DATA_MOONEYE_EMULATOR_ONLY = TESTS_DIR / "testdata" / "mooneye" / "emulator-only"