
import array
from collections import deque
import configparser
from pathlib import Path
import sys
from threading import Lock
from time import perf_counter

import pygame
import sounddevice

from common.python.bindings import GB, LCD, LCD_WIDTH, LCD_HEIGHT, uint8, uint32
from common.python.common import *

BASE_DIR = Path(__file__).parent.parent.parent.resolve()
SCALE = 6.5
KEYBINDS = {}

CONFIG = configparser.ConfigParser()
CONFIG.read(BASE_DIR / "clients" / "pygame" / "config.ini")

def load_keybinds() -> None:
    keybinds = CONFIG["keybinds"]
    KEYBINDS["start"]  = pygame.key.key_code(keybinds.get("start", "return"))
    KEYBINDS["select"] = pygame.key.key_code(keybinds.get("select", "space"))
    KEYBINDS["b"]      = pygame.key.key_code(keybinds.get("b", "a"))
    KEYBINDS["a"]      = pygame.key.key_code(keybinds.get("a", "s"))
    KEYBINDS["down"]   = pygame.key.key_code(keybinds.get("down", "k"))
    KEYBINDS["up"]     = pygame.key.key_code(keybinds.get("up", "i"))
    KEYBINDS["left"]   = pygame.key.key_code(keybinds.get("left", "j"))
    KEYBINDS["right"]  = pygame.key.key_code(keybinds.get("right", "l"))


def load_bootrom() -> None:
    bootrom = None
    rom_config = CONFIG["rom"]
    if rom_config.getboolean("use_bootrom", fallback=False):
        try:
            with open(BASE_DIR / rom_config.get("bootrom", ""), "rb") as f:
                bootrom_data = f.read()
            bootrom = (uint8 * len(bootrom_data)).from_buffer_copy(bootrom_data)
            GB.GB_load_bootrom(bootrom, len(bootrom_data))
        except FileNotFoundError:
            print("Config asks to use bootrom, but could not find file, please edit config.ini. Skipping bootrom")
    if not bootrom:
        GB_set_post_boot_state()


def load_rom() -> None:
    if len(sys.argv) != 2:
        sys.exit(f"Usage: python {sys.argv[0]} <gb rom>")
    rom = sys.argv[1] if sys.argv[1].endswith("gb") else f"{sys.argv[1]}.gb"

    if not (BASE_DIR / rom).exists():
        romlib = CONFIG["rom"].get("romlib", "")
        if not romlib:
            sys.exit("Please set a location for rom library using romlib in config.ini")
        rom = (BASE_DIR / romlib / rom).resolve()
    else:
        rom = (BASE_DIR / rom).resolve()
    try:
        GB_load_rom(rom)
    except FileNotFoundError:
        sys.exit(f"Couldn't find ROM file: {romlib}/{rom}")
        

def window_draw(screen: pygame.Surface, render_surface: pygame.Surface) -> None:
    screen.fill((0, 0, 0))
    dest_rect = render_surface.get_rect().fit(screen.get_rect())
    if not check_blank_frame():
        render_surface = pygame.image.frombuffer(LCD, (LCD_WIDTH, LCD_HEIGHT), "BGRA")
        scaled = pygame.transform.scale(render_surface, dest_rect.size)
        screen.blit(scaled, dest_rect)
    else:
        screen.fill(color="white", rect=dest_rect)


class AudioPlayer:
    # stereo, float32 samples
    BYTES_PER_FRAME = 2 * 4
    MAX_LATENCY_MS = 100

    def __init__(self, samplerate: int) -> None:
        self._lock = Lock()
        self._buffer = bytearray()
        self._max_buffered_bytes = int(samplerate * self.MAX_LATENCY_MS / 1000) * self.BYTES_PER_FRAME
        self._stream = sounddevice.RawOutputStream(
            samplerate=samplerate,
            channels=2,
            dtype="float32",
            callback=self._callback,
        )
        self._stream.start()

    def _callback(self, outdata, frames: int, time, status) -> None:
        """ sounddevice's RawOutputStream expects this argument signature for callback I guess """
        need = frames * self.BYTES_PER_FRAME
        with self._lock:
            have = len(self._buffer)
            take = min(need, have)
            chunk = bytes(self._buffer[:take])
            del self._buffer[:take]
        if take < need:
            print(f"missing {need-take} bytes for audio buffer")
            chunk += b"\x00" * (need - take)
        outdata[:] = chunk

    def push(self, samples: list[float] | None) -> None:
        if not samples:
            return
        with self._lock:
            self._buffer.extend(array.array("f", samples).tobytes())
            excess = len(self._buffer) - self._max_buffered_bytes
            if excess > 0:
                del self._buffer[:excess]

    def close(self) -> None:
        self._stream.stop()
        self._stream.close()


def main() -> None:
    pygame.init()
    screen = pygame.display.set_mode((LCD_WIDTH * SCALE, LCD_HEIGHT * SCALE))
    render_surface = pygame.Surface((LCD_WIDTH, LCD_HEIGHT))

    audio_player = AudioPlayer(AUDIO_SAMPLE_RATE)

    load_keybinds()
    load_bootrom()
    load_rom()
    
    frame_times = deque(maxlen=120)
    frame = 0
    pygame.display.set_caption(f"Gameboy")
    clock = pygame.Clock()

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                audio_player.close()
                print(f"Rendered {frame} frames this session")
                raise SystemExit
        frame_start = perf_counter()

        keys = pygame.key.get_pressed()
        GB.GB_update_joypad(
            not keys[KEYBINDS["start"]],
            not keys[KEYBINDS["select"]],
            not keys[KEYBINDS["a"]],
            not keys[KEYBINDS["b"]],
            not keys[KEYBINDS["down"]],
            not keys[KEYBINDS["up"]],
            not keys[KEYBINDS["left"]],
            not keys[KEYBINDS["right"]]
        )

        GB_emulate_frame()
        output = GB_serial_buffer_flush()
        if output:
            print(f"Serial output: {output}")

        audio_player.push(GB_audio_buffer_flush())
        
        window_draw(screen, render_surface)
        pygame.display.flip()

        frame += 1
        frame_times.append(int((perf_counter() - frame_start) * 1_000_000))
        if frame  % 120 == 0:
            pygame.display.set_caption(f"Gameboy - {sum(frame_times) // len(frame_times):>5} μs/frame")
        clock.tick(59.7275)
            

if __name__ == "__main__":
    main()