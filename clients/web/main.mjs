import createGameBoyModule from "../../build/web/gameboy.mjs";


const GB_RETURN_OK = 0;
const GB_RETURN_RESYNC_VBLANK = 1;

const LCD_WIDTH = 160;
const LCD_HEIGHT = 144;

const FRAME_TIME_MS = 1000.0 / 59.7275
const MAX_CATCHUP_MS = 3 * FRAME_TIME_MS;

const Module = await createGameBoyModule();
const GB_emulate_frame = Module.cwrap('GB_emulate_frame', 'number', []);
const GB_set_post_boot_state = Module.cwrap('GB_set_post_boot_state', 'number', []);
const GB_get_lcd = Module.cwrap('GB_get_lcd', 'number', []);
const GB_update_joypad = Module.cwrap('GB_update_joypad', null,
    ['boolean','boolean','boolean','boolean','boolean','boolean','boolean','boolean']);

const LCDPtr = GB_get_lcd();

const canvas = document.getElementById('canvas-lcd');
canvas.width = LCD_WIDTH;
canvas.height = LCD_HEIGHT;
const ctx = canvas.getContext('2d');
const imageData = ctx.createImageData(LCD_WIDTH, LCD_HEIGHT);

let frameTimer = 0;
let lastTimestamp = null;

function drawFrame() {
    const src = Module.HEAPU8.subarray(LCDPtr, LCDPtr + LCD_HEIGHT * LCD_WIDTH * 4);
    const dst = imageData.data;
    for (let i = 0; i < src.length; i += 4) {
        dst[i]     = src[i + 2]; // R <- B,G,R,A source position 2
        dst[i + 1] = src[i + 1]; // G stays put
        dst[i + 2] = src[i];     // B <- source position 0
        dst[i + 3] = src[i + 3]; // A stays put
    }
    ctx.putImageData(imageData, 0, 0);
}

function loop(timestamp) {
    if (lastTimestamp === null)
        lastTimestamp = timestamp;

    let timeDelta = Math.min(timestamp - lastTimestamp, MAX_CATCHUP_MS);
    lastTimestamp = timestamp;
    frameTimer += timeDelta;

    while (frameTimer >= FRAME_TIME_MS) {
        GB_update_joypad(
            !keyState.start, !keyState.select, !keyState.b, !keyState.a,
            !keyState.down, !keyState.up, !keyState.left, !keyState.right
        );
        GB_emulate_frame();
        frameTimer -= FRAME_TIME_MS;
        drawFrame();
    }

    requestAnimationFrame(loop);
}

async function loadRom(name) {
    const response = await fetch(resolveRomPath(name));
    if (!response.ok) throw new Error(`Failed to fetch ROM "${name}": ${response.status}`);
    const romBytes = new Uint8Array(await response.arrayBuffer());
    const romPtr = Module._malloc(romBytes.length);
    Module.HEAPU8.set(romBytes, romPtr);
    const loadRomFn = Module.cwrap('GB_load_rom', 'number', ['number', 'number']);
    const result = loadRomFn(romPtr, romBytes.length);
    Module._free(romPtr);
    return result;
}

const ROM_ALIASES = {
    'acid2': '../../tests/testdata/dmg-acid2/dmg-acid2.gb',
    'cpu_instrs': '../../tests/testdata/blargg/cpu_instrs/cpu_instrs.gb',
};

function resolveRomPath(name) {
    if (name in ROM_ALIASES)
        return ROM_ALIASES[name];
    return `../../romlib/${name}.gb`;
}

// TODO: sanitize query param
const romName = new URLSearchParams(window.location.search).get('rom');

if (!romName) {
    console.error('No ?rom=<name> query parameter');
} else {
    const result = await loadRom(romName);
    if (result !== GB_RETURN_OK) {
        console.error(`Failed to load ROM "${romName}", GB_load_rom returned ${result}`);
    } else {
        GB_set_post_boot_state();
        requestAnimationFrame(loop);
    }
}

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