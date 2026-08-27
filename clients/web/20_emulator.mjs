/* ############################################################################
###############################################################################
        WASM Bindings
###############################################################################
############################################################################ */

import createGameBoyModule from "../../build/web/gameboy.mjs";
import { get_crc32 } from "./10_main.mjs";
import * as DB from './15_persistence.mjs';

const GB_RETURN_OK = 0;
const GB_MODULE = await createGameBoyModule();

const GB_reboot_system              = GB_MODULE.cwrap('GB_reboot_system', 'number', []);
const GB_set_post_boot_state        = GB_MODULE.cwrap('GB_set_post_boot_state', 'number', []);
const GB_load_rom                   = GB_MODULE.cwrap('GB_load_rom', 'number', ['number', 'number']);
const GB_load_bbram                 = GB_MODULE.cwrap('GB_load_bbram', 'number', ['number', 'number']);
const GB_get_cartridge_ram          = GB_MODULE.cwrap('GB_get_cartridge_ram', 'number', []);
const GB_cartridge_header_as_json   = GB_MODULE.cwrap('GB_cartridge_header_as_json', 'string', []);
const GB_emulate_frame              = GB_MODULE.cwrap('GB_emulate_frame', 'number', []);
const GB_get_lcd                    = GB_MODULE.cwrap('GB_get_lcd', 'number', []);
const GB_audio_buffer_size          = GB_MODULE.cwrap('GB_audio_buffer_size', 'number', []);
const GB_audio_buffer_flush         = GB_MODULE.cwrap('GB_audio_buffer_flush', 'number', []);
const GB_update_joypad              = GB_MODULE.cwrap('GB_update_joypad', null,
    ['boolean','boolean','boolean','boolean','boolean','boolean','boolean','boolean']);
const GB_set_lcd_colors             = GB_MODULE.cwrap('GB_set_lcd_colors', 'number',
    ['number', 'number', 'number', 'number']);

/* ############################################################################
###############################################################################
        Main Loop
###############################################################################
############################################################################ */

let isPaused = true;
let frameTimer = 0;
let lastTimestamp = null;

function loop(timestamp) {
    if (isPaused)
        return;
    if (lastTimestamp === null)
        lastTimestamp = timestamp;

    frameTimer += Math.min(timestamp - lastTimestamp, MAX_CATCHUP_MS);
    lastTimestamp = timestamp;

    while (frameTimer >= FRAME_TIME_MS) {

        // inverted b/c emulator interprets 0 bits as "key pressed"
        GB_update_joypad(
            !keyState.start, !keyState.select, !keyState.b,    !keyState.a,
            !keyState.down,  !keyState.up,     !keyState.left, !keyState.right
        );
        GB_emulate_frame();
        drawFrame();
        queueAudio();
        frameTimer -= FRAME_TIME_MS;
    }
    requestAnimationFrame(loop);
}

// stop loop and audio playback to prevent super annoying audio blips from loop running 1 / sec,
// which seems to be default browser behavior in Firefox (?)
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        isPaused = true;
        // do a last bbram save when user switches tabs, in case they don't come back
        if (currentRom.bbramSize > 0 && currentRom.bbramPtr)
            saveBbram().catch(console.error);
        if (audioCtx)
            audioCtx.suspend();
    } else {
        isPaused = false;
        lastTimestamp = null;
        requestAnimationFrame(loop);
        if (audioCtx)
            audioCtx.resume();
    }
});

/* ############################################################################
###############################################################################
        Display
###############################################################################
############################################################################ */

const LCD_WIDTH = 160;
const LCD_HEIGHT = 144;

const FRAME_TIME_MS = 1000.0 / 59.7275
const MAX_CATCHUP_MS = 3 * FRAME_TIME_MS;

const LCDPtr = GB_get_lcd();

const canvasLcd = document.getElementById('canvas-lcd');
export const canvasHeader = document.getElementById('canvas-header');
canvasLcd.width = LCD_WIDTH;
canvasLcd.height = LCD_HEIGHT;
const ctx = canvasLcd.getContext('2d');
const imageData = ctx.createImageData(LCD_WIDTH, LCD_HEIGHT);

let palette;

export function setPalette(colors) {
    palette = colors;
    GB_set_lcd_colors(palette[0], palette[1], palette[2], palette[3]);
}

function drawFrame() {
    const src = GB_MODULE.HEAPU8.subarray(LCDPtr, LCDPtr + LCD_HEIGHT * LCD_WIDTH * 4);
    imageData.data.set(src);
    ctx.putImageData(imageData, 0, 0);
}

/* ############################################################################
###############################################################################
        Loading ROMs
###############################################################################
############################################################################ */

let currentRom = {};
let showHeaderTimer = null;
let saveBbramInterval = null;
const BBRAM_SAVE_INTERVAL = 10000;

export async function startRom(romBytes, serverRom) {
    // cleanup of previous program state
    currentRom = { crc32: '', title: '', server: false, bbramSize: 0, bbramPtr: null };
    clearInterval(saveBbramInterval);
    GB_reboot_system();

    let loadResult;
    let crc32;
    try {
        const romPtr = GB_MODULE._malloc(romBytes.length);
        GB_MODULE.HEAPU8.set(romBytes, romPtr);
        loadResult = GB_load_rom(romPtr, romBytes.length);
        GB_MODULE._free(romPtr);
        crc32 = get_crc32(romBytes);
    } catch (e) {
        console.error('Failed to load ROM', e);
        return;
    }
    if (loadResult !== GB_RETURN_OK) {
        console.error(`Failed to load ROM with CRC32 hash "${crc32}", GB_load_rom returned ${loadResult}`);
        return;
    }
    const headerJson = JSON.parse(GB_cartridge_header_as_json());
    const bbramSize = headerJson.bbram_numbytes;
    drawHeader(headerJson, crc32);
    setHeaderVisibility(true);
    currentRom = { crc32, title: headerJson.title, server: serverRom, bbramSize };

    // Update IndexedDB lib as needed
    if (await DB.hasLibEntry(crc32))
        await DB.updateLibEntryLastPlayed(crc32);
    else
        await DB.saveLibEntry(crc32, headerJson, romBytes.length, bbramSize);

    if (!serverRom && !(await DB.hasRomData(crc32)))
        await DB.saveRomData(crc32, romBytes);

    if (bbramSize > 0) {
        if (await DB.hasBbramData(crc32)) {
            console.log(`loading existing BBRAM data for cartridge ${crc32}`);
            const bbramData = await DB.loadBbramData(crc32);
            if (bbramData.length == bbramSize) {
                const bbramPtr = GB_MODULE._malloc(bbramSize);
                GB_MODULE.HEAPU8.set(bbramData, bbramPtr);
                GB_load_bbram(bbramPtr, bbramSize);
                GB_MODULE._free(bbramPtr);
            } else
                console.error(`Saved BBRAM size mismatch ${bbramData.length} vs ${bbramSize}, skipping load`);
        }
        currentRom.bbramPtr = GB_get_cartridge_ram();
        saveBbramInterval = setInterval( () => {
            if (!isPaused) saveBbram().catch(console.error);
        }, BBRAM_SAVE_INTERVAL);
    }

    GB_set_post_boot_state();
    GB_set_lcd_colors(palette[0], palette[1], palette[2], palette[3]);

    ctx.clearRect(0, 0, LCD_WIDTH, LCD_HEIGHT);
    frameTimer = 0;
    lastTimestamp = null;
    isPaused = true;

    clearTimeout(showHeaderTimer);
    showHeaderTimer = setTimeout(() => {
        isPaused = false;
        requestAnimationFrame(loop);
        if (!document.getElementById('header-button').matches(':hover'))
            setHeaderVisibility(false);
    }, 2000);
}

async function saveBbram() {
    await DB.saveBbramData(currentRom.crc32,
        GB_MODULE.HEAPU8.subarray(currentRom.bbramPtr, currentRom.bbramPtr + currentRom.bbramSize));
}

// TODO: alternate way to load rom via query param, for direct links?
// const romName = new URLSearchParams(window.location.search).get('rom');
// if (!romName) {
//     console.error('No ?rom=<name> query parameter');
// } else {

/* ############################################################################
###############################################################################
        Display cartridge header info on overlay
###############################################################################
############################################################################ */

export function setHeaderVisibility(visible) {
    if (currentRom.crc32)
        canvasHeader.classList.toggle('visible', visible);
}

function drawHeader(headerJson, crc32) {
    const headerCtx = canvasHeader.getContext('2d');
    const EDGE = 4;

    // need to draw char by char to keep integer coordinates or the precision drifts by later chars
    // and produces antialiasing, which isn't good for the pixelated font
    function drawText(text, x, y) {
        for (const ch of text) {
            headerCtx.fillText(ch, Math.round(x), y);
            x += headerCtx.measureText(ch).width;
        }
    }
    headerCtx.font = '8px "DedicOOL"';
    headerCtx.textBaseline = 'top';
    headerCtx.fillStyle = 'green';
    headerCtx.clearRect(0, 0, LCD_WIDTH, LCD_HEIGHT);
    const byte_ralign = LCD_WIDTH - EDGE - headerCtx.measureText('0x00').width;
    drawText(headerJson.title || '- NO TITLE -', EDGE, EDGE);
    drawText(headerJson.mbc_type, EDGE, 12);
    drawText(headerJson.mbc_type_id, byte_ralign, 12);
    drawText(`CART ROM: ${headerJson.rom_size}`, EDGE, 20);
    drawText(headerJson.rom_size_id, byte_ralign, 20);
    drawText(`CART RAM: ${headerJson.ram_size}`, EDGE, 28);
    drawText(headerJson.ram_size_id, byte_ralign, 28);
    drawText(`CGB: ${headerJson.cgb_flag}`, EDGE, 44);
    drawText(`SGB: ${headerJson.sgb_flag}`, EDGE, 52);
    drawText(`Logo: ${headerJson.logo_ok}`, EDGE, 68);
    drawText(`Checksum header: ${headerJson.checksum_ok}`, EDGE, 76);
    drawText(`Checksum global: ${headerJson.global_checksum_ok}`, EDGE, 84);
    drawText(`CRC32 Hash: ${crc32}`, EDGE, 92);

    drawText(`Destination: ${headerJson.destination}`, EDGE, 108);
    drawText(`Manufacturer: ${headerJson.manufacturer || '-'}`, EDGE, 116);
    drawText(`Licensee: ${headerJson.licensee || '-'}`, EDGE, 124);
    drawText('ROM Version:', EDGE, 132);
    drawText(headerJson.rom_version, byte_ralign, 132);
}

/* ############################################################################
###############################################################################
        Joypad
###############################################################################
############################################################################ */

const keyState = {
    start: false, select: false, b: false, a: false,
    down: false, up: false, left: false, right: false,
};

const KEYMAP = {
    'Enter': 'start',
    'Space': 'select',
    'KeyA': 'b',
    'KeyS': 'a',
    'KeyK': 'down',
    'KeyI': 'up',
    'KeyJ': 'left',
    'KeyL': 'right',
};

window.addEventListener('keydown', (e) => {
    const button = KEYMAP[e.code];
    if (button) {
        keyState[button] = true;
        e.preventDefault();
    }
});
window.addEventListener('keyup', (e) => {
    const button = KEYMAP[e.code];
    if (button) {
        keyState[button] = false;
        e.preventDefault();
    }
});

/* ############################################################################
###############################################################################
        Audio Playback
###############################################################################
############################################################################ */

const AUDIO_SAMPLE_RATE = 44100;
let audioCtx;
let gainNode;
let nextAudioTime = 0;
let gain = 1.0;

// need this to deal with browser policy of not playing audio before some user action on the page
function unlockAudio() {
    if (!audioCtx) {
        audioCtx = new AudioContext({ sampleRate: AUDIO_SAMPLE_RATE });
        gainNode = audioCtx.createGain();
        gainNode.gain.value = gain;
        gainNode.connect(audioCtx.destination);
        audioCtx.resume();
    }
}
window.addEventListener('keydown', unlockAudio, { once: true });
window.addEventListener('click', unlockAudio, { once: true });

function queueAudio() {
    if (!audioCtx)
        return;

    const sampleCount = GB_audio_buffer_size();
    if (sampleCount === 0) return;

    const floatIndex = GB_audio_buffer_flush() / 4; // byte pointer -> HEAPF32 index (4 bytes each)
    const interleaved = GB_MODULE.HEAPF32.subarray(floatIndex, floatIndex + sampleCount * 2); // stereo

    const audioBuffer = audioCtx.createBuffer(2, sampleCount, AUDIO_SAMPLE_RATE);
    const left = audioBuffer.getChannelData(0);
    const right = audioBuffer.getChannelData(1);
    for (let i = 0; i < sampleCount; i++) {
        left[i]  = interleaved[i * 2];
        right[i] = interleaved[i * 2 + 1];
    }

    const source = audioCtx.createBufferSource();
    source.buffer = audioBuffer;
    source.connect(gainNode);

    // playback fell behind — resync with a slight delay, is this enough?
    if (nextAudioTime < audioCtx.currentTime) {
        nextAudioTime = audioCtx.currentTime + 0.05;
    }
    source.start(nextAudioTime);
    nextAudioTime += sampleCount / AUDIO_SAMPLE_RATE;
}

export function setAudioGain(value) {
    gain = value;
    if (gainNode)
        gainNode.gain.value = value;
}
