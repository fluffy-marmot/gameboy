import createGameBoyModule from "../../build/web/gameboy.mjs";

// wasm bindings stuff

const GB_RETURN_OK = 0;

const GB_MODULE = await createGameBoyModule();

const GB_emulate_frame = GB_MODULE.cwrap('GB_emulate_frame', 'number', []);
const GB_set_post_boot_state = GB_MODULE.cwrap('GB_set_post_boot_state', 'number', []);
const GB_load_rom = GB_MODULE.cwrap('GB_load_rom', 'number', ['number', 'number']);
const GB_get_lcd = GB_MODULE.cwrap('GB_get_lcd', 'number', []);
const GB_update_joypad = GB_MODULE.cwrap('GB_update_joypad', null,
    ['boolean','boolean','boolean','boolean','boolean','boolean','boolean','boolean']);
const GB_audio_buffer_size = GB_MODULE.cwrap('GB_audio_buffer_size', 'number', []);
const GB_audio_buffer_flush = GB_MODULE.cwrap('GB_audio_buffer_flush', 'number', []);
const GB_set_lcd_colors = GB_MODULE.cwrap('GB_set_lcd_colors', 'number',
    ['number', 'number', 'number', 'number']);

// main loop

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
            !keyState.start, !keyState.select, !keyState.b, !keyState.a,
            !keyState.down, !keyState.up, !keyState.left, !keyState.right
        );
        GB_emulate_frame();
        drawFrame();
        queueAudio();
        frameTimer -= FRAME_TIME_MS;
    }
    requestAnimationFrame(loop);
}

// display stuff

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

let frameTimer = 0;
let lastTimestamp = null;
let isPaused = false;

function drawFrame() {
    const src = GB_MODULE.HEAPU8.subarray(LCDPtr, LCDPtr + LCD_HEIGHT * LCD_WIDTH * 4);
    imageData.data.set(src);
    ctx.putImageData(imageData, 0, 0);
}

// Loading ROMs

const ROM_ALIASES = {
    'acid2': '../../tests/testdata/dmg-acid2/dmg-acid2.gb',
    'cpu_instrs': '../../tests/testdata/blargg/cpu_instrs/cpu_instrs.gb',
};

function resolveRomPath(name) {
    if (name in ROM_ALIASES)
        return ROM_ALIASES[name];
    return `../../romlib/${name}.gb`;
}

async function loadRom(name) {
    const response = await fetch(resolveRomPath(name));
    if (!response.ok) throw new Error(`Failed to fetch ROM "${name}": ${response.status}`);
    const romBytes = new Uint8Array(await response.arrayBuffer());
    const romPtr = GB_MODULE._malloc(romBytes.length);
    GB_MODULE.HEAPU8.set(romBytes, romPtr);
    const result = GB_load_rom(romPtr, romBytes.length);
    GB_MODULE._free(romPtr);
    return result;
}

// TODO: sanitize query param
const romName = new URLSearchParams(window.location.search).get('rom');

if (!romName) {
    console.error('No ?rom=<name> query parameter');
} else {
    let result
    try {
        result = await loadRom(romName);
    } catch (e) {
        console.error(`Failed to load ROM "${romName}":`, e);
        result = null;
    }

    if (result !== GB_RETURN_OK) {
        console.error(`Failed to load ROM "${romName}", GB_load_rom returned ${result}`);
    } else {
        GB_set_post_boot_state();
        requestAnimationFrame(loop);
    }
}

// Joypad stuff

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

// Audio playback

const AUDIO_SAMPLE_RATE = 44100;
const audioCtx = new AudioContext({ sampleRate: AUDIO_SAMPLE_RATE });
let nextStartTime = 0;
let audioStarted = false;

// need this to deal with browser policy of not playing audio before some user action on the page
function unlockAudio() {
    if (!audioStarted) {
        audioStarted = true;
        audioCtx.resume();
    }
}
window.addEventListener('keydown', unlockAudio, { once: true });
window.addEventListener('click', unlockAudio, { once: true });

function queueAudio() {
    if (!audioStarted)
        return;

    const sampleCount = GB_audio_buffer_size();
    if (sampleCount === 0) return;

    const floatIndex = GB_audio_buffer_flush() / 4; // byte pointer -> HEAPF32 index
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
    source.connect(audioCtx.destination);

    // playback fell behind — resync with a slight delay, is this enough?
    if (nextStartTime < audioCtx.currentTime) {
        nextStartTime = audioCtx.currentTime + 0.05;
    }
    source.start(nextStartTime);
    nextStartTime += sampleCount / AUDIO_SAMPLE_RATE;
}

// stop loop and audio playback to prevent super annoying audio blips from loop running 1 / sec (?)
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        isPaused = true;
        audioCtx.suspend();
    } else {
        isPaused = false;
        lastTimestamp = null;
        audioCtx.resume();
        requestAnimationFrame(loop);
    }
});

// sidebar

let pinned = false;

const toggleButton = document.getElementById('sidebar-toggle');
const sidebar = document.getElementById('sidebar');
const trigger = document.getElementById('sidebar-trigger');

function show() {
    sidebar.classList.add('open');
    updateIcon();
}
function maybeHide() {
    if (!pinned) {
        sidebar.classList.remove('open');
        updateIcon();
    }
}
function updateIcon() {
    toggleButton.innerHTML =
        pinned ? LOCKED_ICON_SVG : sidebar.classList.contains('open') ? UNLOCKED_ICON_SVG : CHEVRON_ICON_SVG;
}

toggleButton.addEventListener('click', () => {
    pinned = !pinned;
    sidebar.classList.toggle('open', pinned);
    updateIcon();
});

toggleButton.addEventListener('mouseenter', show);
toggleButton.addEventListener('mouseleave', maybeHide);
trigger.addEventListener('mouseenter', show);
trigger.addEventListener('mouseleave', maybeHide);
sidebar.addEventListener('mouseenter', show);
sidebar.addEventListener('mouseleave', maybeHide);

// stuff having to do with palette selection divs

document.querySelectorAll('.palette-clr').forEach(el => {
    const gb_color = parseInt(el.dataset.color, 16);
    const r = gb_color & 0xFF;
    const g = (gb_color >> 8) & 0xFF;
    const b = (gb_color >> 16) & 0xFF;
    el.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
});

document.querySelectorAll('.palette-select').forEach(el => {
    el.addEventListener('click', (e) => {
        document.querySelectorAll('.palette-select').forEach(elem => {
            elem.classList.remove('selected');
        });
        el.classList.add('selected');
        const clrs = Array.from(el.querySelectorAll('.palette-clr')).map(p => parseInt(p.dataset.color, 16));
        GB_set_lcd_colors(clrs[0], clrs[1], clrs[2], clrs[3]);
    });
});


// svg icons from bootstrap icons - only using a few so probably worth
// just defining each SVG manually instead of adding a CDN dependency

const CHEVRON_ICON_SVG = `
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" viewBox="0 0 16 16">
<path fill-rule="evenodd" d="M4.146 3.646a.5.5 0 0 0 0 .708L7.793 8l-3.647 3.646a.5.5 0 0 0 .708.708l4-4a.5.5
0 0 0 0-.708l-4-4a.5.5 0 0 0-.708 0M11.5 1a.5.5 0 0 1 .5.5v13a.5.5 0 0 1-1 0v-13a.5.5 0 0 1 .5-.5"/></svg>
`
const UNLOCKED_ICON_SVG = `
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" viewBox="0 0 16 16">
<path fill-rule="evenodd" d="M8 0c1.07 0 2.041.42 2.759 1.104l.14.14.062.08a.5.5 0 0
1-.71.675l-.076-.066-.216-.205A3 3 0 0 0 5 4v2h6.5A2.5 2.5 0 0 1 14 8.5v5a2.5 2.5 0 0 1-2.5 2.5h-7A2.5
2.5 0 0 1 2 13.5v-5a2.5 2.5 0 0 1 2-2.45V4a4 4 0 0 1 4-4M4.5 7A1.5 1.5 0 0 0 3 8.5v5A1.5 1.5 0 0 0 4.5
15h7a1.5 1.5 0 0 0 1.5-1.5v-5A1.5 1.5 0 0 0 11.5 7z"/></svg>
`
const LOCKED_ICON_SVG = `
<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" fill="currentColor" viewBox="0 0 16 16">
<path fill-rule="evenodd" d="M8 0a4 4 0 0 1 4 4v2.05a2.5 2.5 0 0 1 2 2.45v5a2.5 2.5 0 0 1-2.5 2.5h-7A2.5 2.5
0 0 1 2 13.5v-5a2.5 2.5 0 0 1 2-2.45V4a4 4 0 0 1 4-4M4.5 7A1.5 1.5 0 0 0 3 8.5v5A1.5 1.5 0 0 0 4.5 15h7a1.5
1.5 0 0 0 1.5-1.5v-5A1.5 1.5 0 0 0 11.5 7zM8 1a3 3 0 0 0-3 3v2h6V4a3 3 0 0 0-3-3"/></svg>
`

updateIcon();
