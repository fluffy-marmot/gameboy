/* ############################################################################
###############################################################################
        WASM Bindings
###############################################################################
############################################################################ */

import createGameBoyModule from "../../build/web/gameboy.mjs";

const GB_RETURN_OK = 0;
const GB_MODULE = await createGameBoyModule();

const GB_reboot_system          = GB_MODULE.cwrap('GB_reboot_system', 'number', []);
const GB_emulate_frame          = GB_MODULE.cwrap('GB_emulate_frame', 'number', []);
const GB_set_post_boot_state    = GB_MODULE.cwrap('GB_set_post_boot_state', 'number', []);
const GB_load_rom               = GB_MODULE.cwrap('GB_load_rom', 'number', ['number', 'number']);
const GB_get_lcd                = GB_MODULE.cwrap('GB_get_lcd', 'number', []);
const GB_audio_buffer_size      = GB_MODULE.cwrap('GB_audio_buffer_size', 'number', []);
const GB_audio_buffer_flush     = GB_MODULE.cwrap('GB_audio_buffer_flush', 'number', []);
const GB_update_joypad          = GB_MODULE.cwrap('GB_update_joypad', null,
    ['boolean','boolean','boolean','boolean','boolean','boolean','boolean','boolean']);
const GB_set_lcd_colors  = GB_MODULE.cwrap('GB_set_lcd_colors', 'number',
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

const canvas = document.getElementById('canvas-lcd');
canvas.width = LCD_WIDTH;
canvas.height = LCD_HEIGHT;
const ctx = canvas.getContext('2d');
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

async function loadRom(romPath) {
    const response = await fetch(romPath);
    if (!response.ok) throw new Error(`Failed to fetch ROM at "${romPath}": ${response.status}`);
    const romBytes = new Uint8Array(await response.arrayBuffer());
    const romPtr = GB_MODULE._malloc(romBytes.length);
    GB_MODULE.HEAPU8.set(romBytes, romPtr);
    GB_reboot_system();
    const result = GB_load_rom(romPtr, romBytes.length);
    GB_MODULE._free(romPtr);
    return result;
}

export async function startRom(romPath) {
    let result;
    try {
        result = await loadRom(romPath);
    } catch (e) {
        console.error(`Failed to load ROM at "${romPath}":`, e);
        return;
    }
    if (result !== GB_RETURN_OK) {
        console.error(`Failed to load ROM at "${romPath}", GB_load_rom returned ${result}`);
    } else {
        GB_set_post_boot_state();
        GB_set_lcd_colors(palette[0], palette[1], palette[2], palette[3]);
        frameTimer = 0;
        lastTimestamp = null;
        if (isPaused) {
            isPaused = false;
            requestAnimationFrame(loop);
        }
    }
}

// TODO: alternate way to load rom via query param, for direct links?
// const romName = new URLSearchParams(window.location.search).get('rom');
// if (!romName) {
//     console.error('No ?rom=<name> query parameter');
// } else {

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
