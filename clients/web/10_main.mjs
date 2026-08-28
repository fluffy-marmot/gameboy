import './20_emulator.mjs';
import './30_ui.mjs';

// pixelated font for LCD overlay header info display
const dedicoolFont = new FontFace("DedicOOL", "url(dedicool.ttf)");
await dedicoolFont.load();
document.fonts.add(dedicoolFont);

// need to draw char by char to keep integer coordinates or the precision drifts by later chars
// and produces antialiasing, which isn't good for the pixelated font
export function drawTextHelper(ctx, text, x, y) {
    y = Math.round(y);
    for (const ch of text) {
        ctx.fillText(ch, Math.round(x), y);
        x += ctx.measureText(ch).width;
    }
}

// CRC32 algorithm by Claude
let CRC32_TABLE;
export function get_crc32(bytes) {
    if (!CRC32_TABLE) {
        CRC32_TABLE = new Uint32Array(256);
        for (let n = 0; n < 256; n++) {
            let c = n;
            for (let k = 0; k < 8; k++)
                c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
            CRC32_TABLE[n] = c >>> 0;
        }
    }
    let crc = 0xFFFFFFFF;
    for (let i = 0; i < bytes.length; i++)
        crc = CRC32_TABLE[(crc ^ bytes[i]) & 0xFF] ^ (crc >>> 8);
    return ((crc ^ 0xFFFFFFFF) >>> 0).toString(16).toUpperCase().padStart(8, '0');
}
