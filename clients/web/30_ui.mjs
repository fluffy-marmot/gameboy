
import {
    startRom, setPalette, setAudioGain, setHeaderVisibility
} from "./20_emulator.mjs";
import {
    ROM_TREE,
    CHEVRON_ICON_SVG, PIN_ICON_SVG, PIN_FILL_ICON_SVG,
    VOLUME_UP_SVG, VOLUME_DOWN_SVG, VOLUME_OFF_SVG, VOLUME_MUTE_SVG,
    HEADER_CARD_SVG
} from "./40_assets.mjs";

/* ############################################################################
###############################################################################
        Sidebar show/hide and pin toggle button
###############################################################################
############################################################################ */

let pinned = false;

const toggleButton = document.getElementById('sidebar-toggle');
const sidebar = document.getElementById('sidebar');
const trigger = document.getElementById('sidebar-trigger');

function showSidebar() {
    sidebar.classList.add('open');
    updateToggleIcon();
}
function maybeHideSidebar() {
    if (!pinned) {
        sidebar.classList.remove('open');
        updateToggleIcon();
    }
}
function updateToggleIcon() {
    toggleButton.innerHTML =
        pinned ? PIN_FILL_ICON_SVG : sidebar.classList.contains('open') ? PIN_ICON_SVG : CHEVRON_ICON_SVG;
}
updateToggleIcon();

toggleButton.addEventListener('click', () => {
    pinned = !pinned;
    sidebar.classList.toggle('open', pinned);
    updateToggleIcon();
});

toggleButton.addEventListener('mouseenter', showSidebar);
toggleButton.addEventListener('mouseleave', maybeHideSidebar);
trigger.addEventListener('mouseenter', showSidebar);
trigger.addEventListener('mouseleave', maybeHideSidebar);
sidebar.addEventListener('mouseenter', showSidebar);
sidebar.addEventListener('mouseleave', maybeHideSidebar);

/* Icon that shows header info when hovering mouse */

const headerButton = document.getElementById('header-button');
headerButton.innerHTML = HEADER_CARD_SVG;

headerButton.addEventListener('mouseenter', () => { setHeaderVisibility(true);  });
headerButton.addEventListener('mouseleave', () => { setHeaderVisibility(false); });

/* ############################################################################
###############################################################################
        Volume control
###############################################################################
############################################################################ */

const volumeIcon = document.getElementById('volume-icon');
const volumeSlider = document.getElementById('volume-slider');

let volume = parseFloat(volumeSlider.value);
let muted = false;

function updateVolume() {
    volumeIcon.innerHTML = muted ? VOLUME_MUTE_SVG :
    volume < 0.05 ? VOLUME_OFF_SVG : volume < 0.6 ? VOLUME_DOWN_SVG : VOLUME_UP_SVG;
    setAudioGain(muted ? 0.0 : volume);
}
updateVolume();

volumeIcon.addEventListener('click', (e) => {
    muted = !muted;
    volumeSlider.disabled = muted;
    volumeSlider.value = muted ? 0.0 : volume;

    updateVolume();
});

volumeSlider.addEventListener('input', (e) => {
    volume = parseFloat(volumeSlider.value);
    updateVolume();
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

document.querySelectorAll('.palette-select').forEach(el => {
    el.addEventListener('click', (e) => {
        document.querySelectorAll('.palette-select').forEach(elem => {
            elem.classList.remove('selected');
        });
        el.classList.add('selected');
        useSelectedPalette();
    });
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
        nodeDiv.classList.add('rom-leaf');
        nodeDiv.textContent = name;
        nodeDiv.dataset.path = node.path;
        return nodeDiv;
    }
}

// Build tree based on ROM_TREE
const romsDiv = document.querySelector('#sidebar-roms-content');
Object.entries(ROM_TREE.children).forEach(([name, node]) => {
    const categoryNode = renderRomTreeNode(name, node);
    categoryNode.style.marginLeft = 0;
    romsDiv.append(categoryNode);
});

romsDiv.addEventListener('click', (e) => {
    const leaf = e.target.closest('.rom-leaf');
    if (!leaf) return;
    startRom(leaf.dataset.path);
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
