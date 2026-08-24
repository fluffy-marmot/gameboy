# Gameboy Emulator

This is a work in progress implementation of a Gameboy emulator. Kind of a fun project to have an excuse to learn more about C, which I enjoy, and have fun programming something low-level.

### What's done

- CPU instruction set
- PPU (Picture processing unit), the GPU of the Gameboy
- APU (audio processing unit)
- Interrupt system
- DMA (Direct memory access), a fast memory transfer system that moves a whopping whole 160 bytes (wowee!)
- Joypad controller interactions
- Support for basic cartridge and MBC1 type memory bank controller cartridges
- Pytest automated tests of a few popular test suites: SM83 CPU instruction set, acid-test2, Blargg, mooneye,
  and mealybug (many of the stricter / quirky edge case tests still not passing). This requires downloading the
  testdata separately from their respective repositories

### To do
- Support more various cartridge types
- Improve various timing inaccuracies and small bugs, especially with PPU rendering
- maybe some kind of web client, with an Emscripten-compiled version of the emulator?

Currently it allows you to play the basic cartridge games (some examples useful for testing are Dr. Mario and
Tetris) and MBC1 cartridges (Legend of Zelda: Link's Awakening, Prehistorik Man)

## Running

If you want to mess around with this yourself, on Linux:

Compile the C code into a shared object library (this gets output in `build/`)
```bash
make lib
```
There's a Python-based client that can use this .so file that handles the actual graphics rendering, audio
playing, and user input handling. It needs a couple of packages listed in
[requirements.txt](/clients/pygame/requirements.txt)...set that up however you prefer, for example:
```bash
pip install -r clients/pygame/requirements.txt
```
You can change your preferred keybindings and location of your Gameboy ROM programs in
[config.ini](/clients/pygame/config.ini) (to use the default location, place
ROMs in a subdirectory called `romlib`). Unfortunately the games are copyrighted so examples cannot be
included
along with the emulator.

Once you have a Gameboy ROM program in the ROM library, for example `romlib/zelda.gb`, you can run it using
```bash
make run ROM=zelda
```
