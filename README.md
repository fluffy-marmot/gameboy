### Gameboy Emulator

This is a work in progress implementation of a Gameboy emulator. Kind of a fun project to have an excuse to use
and learn more about C, which I enjoy, and have fun programming something low-level. 

What's done:
- CPU instruction set
- PPU (Picture processing unit), kind of a primitive GPU of the Gameboy
- Interrupt system
- DMA (Direct memory access), a fast memory transfer system that moves a whopping whole 160 bytes (wowee!)
- Joypad controller interactions
- Support for the most basic cartridge type only

To do:
- Support more various cartridges (the ones that use embedded RAM, ROM banking systems)
- Audio (currently doesn't work at all)
- Better debugging / testing interface and implementation
- maybe some kind of web client, with an Emscripten-compiled version of the emulator?

Currently it allows you to play the basic cartridge games (some examples useful for testing are Dr. Mario and
Tetris) without audio using a temporary and messy hacked together Pygame wrapper, without working audio.