from common.python.bindings import *
from common.python.common import *
from tests.config import *

from PIL import Image, ImageChops
import pytest

def run_image_test(testrom: Path, total_pixels: dict | None):
    GB_load_rom(testrom)
    GB_emulate_until_opcode(0x40)

    output_path = IMG_TEST_OUTPUT / f"{testrom.stem}.png"
    img_test_output(output_path)

    reference_path = testrom.parent / f"{testrom.stem}.png"
    if not reference_path.exists():
        pytest.fail(f"{testrom.stem}: missing reference png", pytrace=False)

    output = Image.open(output_path).convert("RGBA")
    reference = Image.open(reference_path).convert("RGBA")

    if output.size != reference.size:
        pytest.fail(f"{testrom.stem}: reference image size mismatch", pytrace=False)

    diff = ImageChops.difference(output, reference)
    diff_count = 0
    for x in range(LCD_WIDTH):
        for y in range(LCD_HEIGHT):
            if diff.getpixel((x, y)) != (0, 0, 0, 0):
                diff_count += 1
                r, _, _, a = output.getpixel((x, y))
                output.putpixel((x, y), (r, 0, 0, a))
    output.save(output_path)
    if total_pixels:
        total_pixels["total"] += diff_count

    bbox = diff.getbbox(alpha_only=False)
    if diff_count != 0:
        pytest.fail(
            f"{testrom.stem}: Diff {diff_count}/{LCD_WIDTH * LCD_HEIGHT}px Bound: {bbox}", pytrace=False)


def img_test_output(output_path: Path):
    img = Image.frombuffer("RGBA", (LCD_WIDTH, LCD_HEIGHT), LCD, "raw", "BGRA", 0, 1)
    img.save(output_path)