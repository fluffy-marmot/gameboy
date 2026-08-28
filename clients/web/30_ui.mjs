import * as DB from './15_persistence.mjs';
import {
    startRom, setPalette, setAudioGain, setHeaderVisibility
} from "./20_emulator.mjs";
import {
    ROM_TREE,
    CHEVRON_ICON_SVG, PIN_ICON_SVG, PIN_FILL_ICON_SVG,
    VOLUME_UP_SVG, VOLUME_DOWN_SVG, VOLUME_OFF_SVG, VOLUME_MUTE_SVG,
    HEADER_CARD_SVG, UPLOAD_FILE_SVG
} from "./40_assets.mjs";

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
}
function maybeHideSidebar() {
    if (!DB.settings.pinned) {
        sidebar.classList.remove('open');
        updateToggleIcon();
    }
}
function updateToggleIcon() {
    toggleButton.innerHTML = DB.settings.pinned ?
        PIN_FILL_ICON_SVG : (sidebar.classList.contains('open') ? PIN_ICON_SVG : CHEVRON_ICON_SVG);
    toggleButton.ariaPressed = DB.settings.pinned ? 'true' : 'false';
}

toggleButton.addEventListener('click', () => {
    DB.updateSetting('pinned', !DB.settings.pinned);
    sidebar.classList.toggle('open', DB.settings.pinned);
    updateToggleIcon();
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

/* Icon that shows header info when hovering mouse */
const headerButton = document.getElementById('header-button');
headerButton.innerHTML = HEADER_CARD_SVG;
headerButton.addEventListener('mouseenter', () => { setHeaderVisibility(true);  });
headerButton.addEventListener('mouseleave', () => { setHeaderVisibility(false); });

/* Upload ROM file button that triggers file input */
const uploadRomButton = document.getElementById('upload-rom-button');
uploadRomButton.innerHTML = UPLOAD_FILE_SVG;
uploadRomButton.addEventListener('click', () => { uploadRomInput.click(); });

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
    volumeIcon.innerHTML = DB.settings.muted ? VOLUME_MUTE_SVG :
        volume < 0.05 ? VOLUME_OFF_SVG : volume < 0.6 ? VOLUME_DOWN_SVG : VOLUME_UP_SVG;
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
        Palette selection stuff
###############################################################################
############################################################################ */

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
        ROM Selection Tree
###############################################################################
############################################################################ */

// On load, Build ROM Tree of available user, server, and test ROMs
const libraryUser = document.querySelector('#library-user');
const libraryServer = document.querySelector('#library-server');
const libraryTests = document.querySelector('#library-tests');
const rom_containers = { libraryServer, libraryTests };

// fill in user ROMs based on contents of IndexedDB
libraryUser.style.display = 'none';
(await DB.loadLib()).forEach(async libEntry => {
    if (await DB.hasRomData(libEntry.crc32)) {
        const nodeDiv = document.createElement('div');
        nodeDiv.classList.add('rom-leaf', 'user-rom-leaf');
        nodeDiv.textContent = libEntry.title;
        nodeDiv.dataset.crc32 = libEntry.crc32;
        libraryUser.append(nodeDiv);
        libraryUser.style.display = 'block';
    }
});

// recursively fill in server and test ROMs into categories
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
Object.entries(ROM_TREE.children).forEach(([libName, lib]) => {
    Object.entries(lib.children).forEach(([name, node]) => {
        rom_containers[libName].append(renderRomTreeNode(name, node));
    });
});

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

// Handle ROM file uploads
const uploadRomInput = document.querySelector('#upload-rom-input');
uploadRomInput.addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;

    const romBytes = new Uint8Array(await file.arrayBuffer());
    // TODO add a new leaf for the rom...? need proper ordering
    startRom(romBytes, false);
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
