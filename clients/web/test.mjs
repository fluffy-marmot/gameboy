import createGameboyModule from '../../build/web/gameboy.mjs';
const Module = await createGameboyModule();

import { readFileSync } from 'node:fs';
const romBytes = readFileSync('tests/testdata/blargg/cpu_instrs/individual/05-op rp.gb');

const rom_ptr = Module._malloc(romBytes.length);
Module.HEAPU8.set(romBytes, rom_ptr);

Module.ccall('GB_load_rom', 'number', ['number', 'number'], [rom_ptr, romBytes.length]);

for (let i = 0; i < 1200; i++)
    Module.ccall('GB_emulate_frame');

const serial_buffer_size = Module.ccall('GB_serial_buffer_size', 'number', [], []);
const serial_buffer_ptr = Module.ccall('GB_serial_buffer_flush', 'number', [], []);
const serial_output = Module.HEAPU8.subarray(serial_buffer_ptr, serial_buffer_ptr + serial_buffer_size);

console.log(new TextDecoder('ascii').decode(serial_output));
