import array
import ctypes
import math
import pathlib
import sys

import numpy as np
import pygame

from gameboy_cstuff import CPU, cpu, GB, LCD

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

    f = open("gb.log", "w")
    pygame.init()
    screen = pygame.display.set_mode((160, 144))
    GB.set_post_boot_state()
    f.write(f"A:{cpu.A:02X} F:{cpu.F:02X} B:{cpu.B:02X} C:{cpu.C:02X} D:{cpu.D:02X} E:{cpu.E:02X} H:{cpu.H:02X} L:{cpu.L:02X} SP:{cpu.SP:04X} PC:{cpu.PC:04X} PCMEM:{GB.read_bus_dispatch(cpu.PC):02X},{GB.read_bus_dispatch(cpu.PC + 1):02X},{GB.read_bus_dispatch(cpu.PC +2):02X},{GB.read_bus_dispatch(cpu.PC+3):02X}\n")
    GB.fetch_instruction()

    frame = 0
    dot = 0
    pygame.display.set_caption(f"Gameboy")
    clock = pygame.Clock()
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                raise SystemExit
            elif event.type in [pygame.KEYDOWN, pygame.KEYUP]:
                pass

        for _ in range(456 * 154 // 4):
            result = GB.tick_machine_cycle()
            print(f"{cpu.PC:#X}  --  {result & 0xFF:#X}")
            if (result >> 8 == 0):
                f.write(f"A:{cpu.A:02X} F:{cpu.F:02X} B:{cpu.B:02X} C:{cpu.C:02X} D:{cpu.D:02X} E:{cpu.E:02X} H:{cpu.H:02X} L:{cpu.L:02X} SP:{cpu.SP:04X} PC:{cpu.PC:04X} PCMEM:{GB.read_bus_dispatch(cpu.PC):02X},{GB.read_bus_dispatch(cpu.PC + 1):02X},{GB.read_bus_dispatch(cpu.PC +2):02X},{GB.read_bus_dispatch(cpu.PC+3):02X}\n")
            for _ in range(4):
                GB.dot_cycle()
                dot += 1

        window_draw(screen)
        pygame.display.flip()

        clock.tick(60)
        frame += 1

if __name__ == "__main__":
    main()