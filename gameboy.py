import array
import ctypes
import math
import pathlib
import sys

import numpy as np
import pygame

from gameboy_cstuff import GB, LCD

def window_draw(screen: pygame.Surface) -> None:
    np_view = np.ctypeslib.as_array(LCD, shape=(144, 160))

    pygame.surfarray.blit_array(screen, np_view.T)
    

def main() -> None:
    # if len(sys.argv) != 2:
    #     sys.exit(f"Usage: python {sys.argv[0]} <.ch8 program>")
    # elif not pathlib.Path(sys.argv[1]).exists():
    #     sys.exit(f"Cannot find file {sys.argv[1]}")
    # try:
    #     CHIP8 = ctypes.CDLL("./chip8.so")
    #     CHIP8.load_program(sys.argv[1].encode())
    #     CHIP8.get_display.restype = ctypes.POINTER(Display)
    #     DISPLAY = CHIP8.get_display().contents

    #     win_w, win_h = get_window_dimensions()
    # except Exception as e:
    #     print(e)
    #     sys.exit(1)

    pygame.init()
    screen = pygame.display.set_mode((160, 144))


    pygame.display.set_caption(f"Gameboy")
    clock = pygame.Clock()
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                raise SystemExit
            elif event.type in [pygame.KEYDOWN, pygame.KEYUP] and event.scancode in KEYMAP:
                pass

        for _ in range(456 * 154 // 4):
            GB.tick_machine_cycle()
            for _ in range(4):
                GB.dot_cycle()

        window_draw(screen)
        pygame.display.flip()

        clock.tick(60)

if __name__ == "__main__":
    main()