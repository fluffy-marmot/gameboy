import { drawTextHelper } from './10_main.mjs';
import * as DB from './15_persistence.mjs';
import {
    LCD_WIDTH, LCD_HEIGHT, gbJoypad,
    startRom, setPalette, setAudioGain, setHeaderVisibility
} from "./20_emulator.mjs";
import * as SVG from "./40_assets.mjs";

/* ############################################################################
###############################################################################
        Sidebar show/hide and pin toggle button
###############################################################################
############################################################################ */

const toggleButton = document.getElementById('sidebar-toggle');
const sidebar = document.getElementById('sidebar');
const trigger = document.getElementById('sidebar-trigger');

function showSidebar() {
    sidebar.classList.add('open');
    updateToggleIcon();
    toggleButton.classList.add('fs-visible');
}
function maybeHideSidebar() {
    if (!DB.settings.pinned) {
        sidebar.classList.remove('open');
        updateToggleIcon();
    }
}
function updateToggleIcon() {
    toggleButton.classList.toggle('fs-visible', DB.settings.pinned);
    toggleButton.innerHTML = DB.settings.pinned ?
        SVG.PIN_FILL_ICON : (sidebar.classList.contains('open') ? SVG.PIN_ICON : SVG.CHEVRON_ICON);
    toggleButton.ariaPressed = DB.settings.pinned ? 'true' : 'false';
}

toggleButton.addEventListener('click', () => {
    DB.updateSetting('pinned', !DB.settings.pinned);
    showSidebar();
});

toggleButton.addEventListener('mouseenter', showSidebar);
toggleButton.addEventListener('mouseleave', maybeHideSidebar);
trigger.addEventListener('mouseenter', showSidebar);
trigger.addEventListener('mouseleave', maybeHideSidebar);
sidebar.addEventListener('mouseenter', showSidebar);
sidebar.addEventListener('mouseleave', maybeHideSidebar);

// for initial look, based on current settings
if (DB.settings.pinned)
    showSidebar();
updateToggleIcon()

// Icon that shows header info when hovering mouse
const headerButton = document.getElementById('header-button');
headerButton.innerHTML = SVG.HEADER_CARD;
headerButton.addEventListener('mouseenter', (e) => { setHeaderVisibility(true);  });
headerButton.addEventListener('mouseleave', (e) => { setHeaderVisibility(false); });

// Upload ROM file button that triggers file input
const uploadRomButton = document.getElementById('upload-rom-button');
uploadRomButton.innerHTML = SVG.UPLOAD_FILE;
uploadRomButton.addEventListener('click', (e) => { uploadRomInput.click(); });

const fullscreenButton = document.getElementById('fullscreen-button');
fullscreenButton.innerHTML = SVG.FULLSCREEN;
fullscreenButton.addEventListener('click', (e) => {
    if (document.fullscreenElement)
        document.exitFullscreen();
    else
        document.body.requestFullscreen().catch(console.error);
});

const githubButton = document.getElementById('github-button');
githubButton.innerHTML = SVG.GITHUB;

// Set initial expanded / collapsed state of main sidebar sections, based on settings
document.getElementById('sidebar-main-content').querySelectorAll(':scope > details, :scope > div > details')
.forEach(section => {
    section.open = DB.settings.sectionOpen[section.id];
    section.addEventListener('toggle', (e) => {
        DB.settings.sectionOpen[section.id] = section.open
        DB.updateSetting('sectionOpen', DB.settings.sectionOpen);
    });
});

/* ############################################################################
###############################################################################
        Volume control
###############################################################################
############################################################################ */

const volumeIcon = document.getElementById('volume-icon');
const volumeSlider = document.getElementById('volume-slider');

// keep this value separate from DB.settings becasuse slider input event can spam rapid changes
let volume = DB.settings.volume;
volumeSlider.value = DB.settings.muted ? 0.0 : volume;

function updateVolume() {
    volumeSlider.disabled = DB.settings.muted;
    volumeIcon.innerHTML = DB.settings.muted ? SVG.VOLUME_MUTE :
        volume < 0.05 ? SVG.VOLUME_OFF : volume < 0.6 ? SVG.VOLUME_DOWN : SVG.VOLUME_UP;
    setAudioGain(DB.settings.muted ? 0.0 : volume);
}
updateVolume();

volumeIcon.addEventListener('click', (e) => {
    DB.updateSetting('muted', !DB.settings.muted);
    volumeSlider.value = DB.settings.muted ? 0.0 : volume;
    updateVolume();
});

volumeSlider.addEventListener('input', (e) => {
    volume = parseFloat(volumeSlider.value);
    updateVolume();
});

volumeSlider.addEventListener('change', (e) => {
    DB.updateSetting('volume', volume);
});

/* ############################################################################
###############################################################################
        Joypad Controls Editing
###############################################################################
############################################################################ */

let keyEditControl;
const canvasInfo = document.querySelector('.canvas-info');
const infoCtx = canvasInfo.getContext('2d');
infoCtx.fillStyle = 'green';

function drawCenterText(text, y) {
    drawTextHelper(infoCtx, text, (LCD_WIDTH - infoCtx.measureText(text).width) / 2, y);
}

function updateKeyBindings() {
    gbJoypad.keyState = {
        start: false, select: false, b: false, a: false,
        down: false, up: false, left: false, right: false,
    };
    gbJoypad.keyMap = {};
    for (const [control, keyBind] of Object.entries(DB.settings.joypad)) {
        if (keyBind)
            gbJoypad.keyMap[keyBind] = control;
        document.querySelector(`.joypad-row[data-control="${control}"] .joypad-key-pill`).textContent = keyBind;
    }
}
updateKeyBindings();

document.querySelectorAll('.joypad-update').forEach(button => {
    button.addEventListener('click', (e) => {
        gbJoypad.keyEditMode = true;
        keyEditControl = button.closest('.joypad-row').dataset.control;
        canvasInfo.classList.add('visible');

        infoCtx.clearRect(0, 0, LCD_WIDTH, LCD_HEIGHT);

        infoCtx.font = '8px "DedicOOL"';
        drawCenterText('Press new key for', LCD_HEIGHT / 3);
        infoCtx.font = '24px "DedicOOL"';
        drawCenterText(keyEditControl, LCD_HEIGHT * 2 / 3);
    });
});

window.addEventListener('keydown', (e) => {
    if (!gbJoypad.keyEditMode) return;
    e.preventDefault();
    if (e.code !== 'Escape') {
        for (const [control, keyBind] of Object.entries(DB.settings.joypad)) {
        if (keyBind == e.code)
            DB.settings.joypad[control] = null;
        }
        DB.settings.joypad[keyEditControl] = e.code;
        DB.updateSetting('joypad', DB.settings.joypad);
        updateKeyBindings();
    }
    canvasInfo.classList.remove('visible');
    gbJoypad.keyEditMode = false;
});

document.querySelectorAll('.joypad-update').forEach(el => {
    el.innerHTML = SVG.PENCIL_EDIT;
});

/* ############################################################################
###############################################################################
        Palette selection stuff
###############################################################################
############################################################################ */

const colorHelper = (clr) => `FF${clr.toString(16).toUpperCase().padStart(6, '0')}`;

// set up the custom (4th) palette and customize it via query parameters if needed
const params = new URLSearchParams(window.location.search);
const customPalette = [...DB.settings.customPalette];
let colorImported = false;
for (let i = 0; i < 4; i++) {
    const importedClr = parseInt(params.get(`color${i}`), 16);
    if (!isNaN(importedClr)) {
        colorImported = true;
        customPalette[i] = importedClr & 0xFFFFFF;
    }
    document.getElementById(`custom-clr${i}`).dataset.color = colorHelper(customPalette[i]);
}
if (colorImported) {
    DB.updateSetting('customPalette', customPalette);
    DB.updateSetting('palette', 3);
}

// on page load, set colors from data attributes, the format is friendlier for other things
document.querySelectorAll('.palette-clr').forEach(el => {
    const gb_color = parseInt(el.dataset.color, 16);
    const r = gb_color & 0xFF;
    const g = (gb_color >> 8) & 0xFF;
    const b = (gb_color >> 16) & 0xFF;
    el.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
});

document.querySelectorAll('.palette-select').forEach((el, idx) => {
    el.addEventListener('click', (e) => {
        document.querySelectorAll('.palette-select').forEach(elem => {
            elem.classList.remove('selected');
        });
        el.classList.add('selected');
        useSelectedPalette();
        DB.updateSetting('palette', idx);
    });
    el.classList.toggle('selected', idx == DB.settings.palette);
});

function useSelectedPalette() {
    const palette = document.querySelector('.palette-select.selected');
    if (!palette) return;
    setPalette(Array.from(palette.querySelectorAll('.palette-clr')).map(p => parseInt(p.dataset.color, 16)));
}
useSelectedPalette();

/* ############################################################################
###############################################################################
        ROM Selection Trees
###############################################################################
############################################################################ */

// On load, Build ROM Tree of available user, server, and test ROMs
const libraryUser = document.querySelector('#section-lib-user');
const libraryServer = document.querySelector('#section-lib-server');
const libraryTests = document.querySelector('#section-lib-tests');
const rom_containers = { libraryServer, libraryTests };

const titleCase = (str) => str.toLowerCase().replace(/\b\w/g, c => c.toUpperCase());

// fill in user ROMs based on contents of IndexedDB
libraryUser.style.display = 'none';

function addUserLibRom(title, crc32) {
    const nodeDiv = document.createElement('div');
    nodeDiv.classList.add('rom-leaf', 'user-rom-leaf');
    nodeDiv.textContent = titleCase(title);
    nodeDiv.dataset.crc32 = crc32;
    libraryUser.append(nodeDiv);
    libraryUser.style.display = 'block';
}
(await DB.loadLib()).forEach(async libEntry => {
    if (await DB.hasRomData(libEntry.crc32)) {
        addUserLibRom(libEntry.title, libEntry.crc32);
    }
});

const serverRoms = { libraryServer: {}, libraryTests: {} };
async function getServerRomsManifests() {
    // start a fetch for the tests manifest that is always used
    const testsPromise = fetch('_roms-tests.json');

    // potentially load an extra server-side library provided by a query parameter
    // if one has ever been provided, it's also saved to persistent settings
    const importLib = params.get('load-library');
    if (importLib && !importLib.includes('://') && !importLib.startsWith('//'))
        DB.updateSetting('extraServerLibrary', params.get('load-library'));

    if (DB.settings.extraServerLibrary) {
        const libResponse = await fetch(DB.settings.extraServerLibrary, { cache: 'no-cache' });
        if (!libResponse.ok)
            console.log(`Failed to fetch ${DB.settings.extraServerLibrary} manifest: ${libResponse.status}`);
        else
            serverRoms.libraryServer = await libResponse.json();
    }

    const testsResponse = await testsPromise;
    if (!testsResponse.ok)
        console.log(`Failed to fetch test roms manifest: ${testsResponse.status}`);
    else
        serverRoms.libraryTests = await testsResponse.json();
}
await getServerRomsManifests();

// helper for recursively filling in server and test ROMs into categories
function renderRomTreeNode(name, node) {
    if (node.type == 'dir') {
        const nodeDetails = document.createElement('details');
        const nodeSummary = document.createElement('summary');
        nodeSummary.textContent = name;
        nodeDetails.append(nodeSummary);
        Object.entries(node.children).forEach(([childName, child]) => {
            nodeDetails.append(renderRomTreeNode(childName, child));
        });
        return nodeDetails;
    } else {
        const nodeDiv = document.createElement('div');
        nodeDiv.classList.add('rom-leaf', 'server-rom-leaf');
        nodeDiv.textContent = name;
        nodeDiv.dataset.path = node.path;
        return nodeDiv;
    }
}

// server and test ROM categories
Object.entries(serverRoms).forEach(([libName, lib]) => {
    if (!lib.children) {
        rom_containers[libName].style.display = 'none';
        return;
    }
    Object.entries(lib.children).forEach(([name, node]) => {
        rom_containers[libName].append(renderRomTreeNode(name, node));
    });
});

// launch a ROM when clicked
const romsDiv = document.querySelector('#sidebar-roms-content');
romsDiv.addEventListener('click', async (e) => {
    const leaf = e.target.closest('.rom-leaf');
    if (!leaf) return;
    let romBytes = null;
    if (leaf.classList.contains('server-rom-leaf')) {
        const response = await fetch(leaf.dataset.path);
        if (!response.ok) {
            console.log(`Failed to fetch ROM at "${leaf.dataset.path}": ${response.status}`);
            return;
        }
        romBytes = new Uint8Array(await response.arrayBuffer());
        startRom(romBytes, true);
    } else if (leaf.classList.contains('user-rom-leaf')) {
        romBytes = await DB.loadRomData(leaf.dataset.crc32);
        startRom(romBytes, false);
    }
});

/* ############################################################################
###############################################################################
        ROM Uploads
###############################################################################
############################################################################ */

// reject uploads above this size, no point in trying to process large files
const MAX_ROM_SIZE = 8 * 1024 * 1024;

async function launchRomFile(file) {
    if (!file) return;
    if (file.size > MAX_ROM_SIZE) {
        console.log(`"${file.name}": ${file.size} bytes exceeds max ROM size of ${MAX_ROM_SIZE} bytes`);
        return;
    }
    const romBytes = new Uint8Array(await file.arrayBuffer());
    const result = await startRom(romBytes, false);
    if (result && result.crc32 && !document.querySelector(`.user-rom-leaf[data-crc32="${result.crc32}"]`))
        addUserLibRom(result.title, result.crc32);
}

const uploadRomInput = document.querySelector('#upload-rom-input');
uploadRomInput.addEventListener('change', async (e) => {
    launchRomFile(e.target.files[0]).catch(console.error);
});

// Handle drag-and-drop file uploads in same place while we're at it
const lcdContainer = document.getElementById('lcd-container');

lcdContainer.addEventListener('dragenter', (e) => {
    e.preventDefault();
    gbJoypad.keyEditMode = false;
    canvasInfo.classList.add('visible');
    infoCtx.clearRect(0, 0, LCD_WIDTH, LCD_HEIGHT);
    infoCtx.font = '16px "DedicOOL"';
    drawCenterText('Import ROM', LCD_HEIGHT / 2);
    infoCtx.font = '32px "DedicOOL"';
    drawCenterText('↑', LCD_HEIGHT * 3 / 4);
});

lcdContainer.addEventListener('dragover', (e) => {
    e.preventDefault();
});

lcdContainer.addEventListener('dragleave', (e) => {
    canvasInfo.classList.remove('visible');
});

lcdContainer.addEventListener('drop', async (e) => {
    e.preventDefault();
    canvasInfo.classList.remove('visible');
    launchRomFile(e.dataTransfer.files[0]).catch(console.error);
});

/* ############################################################################
###############################################################################
        Tooltip
###############################################################################
############################################################################ */

// ROM names, especially tests, are long and overflow the menu
const tooltip = document.createElement('div');
tooltip.id = 'rom-tooltip';
document.body.append(tooltip);

romsDiv.addEventListener('mouseover', (e) => {
    const leaf = e.target.closest('.rom-leaf');
    if (!leaf) {
        tooltip.style.display = 'none';
        return;
    }
    const rect = leaf.getBoundingClientRect();
    tooltip.textContent = leaf.textContent;
    tooltip.style.font = getComputedStyle(leaf).font;
    tooltip.style.left = `${rect.left}px`;
    tooltip.style.top = `${rect.top}px`;
    tooltip.style.display = 'block';
});
romsDiv.addEventListener('mouseout', (e) => {
    if (!romsDiv.contains(e.relatedTarget)) tooltip.style.display = 'none';
});
