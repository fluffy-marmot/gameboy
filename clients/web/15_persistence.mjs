/* ############################################################################
###############################################################################
        IndexedDB for saving ROM lib metadata, ROMs, BBRAM, bootroms
###############################################################################
############################################################################ */
/*
schema:

library : crc32, title, header, romSize, bbramSize, dateAdded, dateLastPlayed, dbVersion
roms    : crc32, data
bbram   : crc32, data
bootroms: id, name, data - not added yet, but no need to update version later
*/

const DB_LIBRARY  = 'library';
const DB_ROMS     = 'roms';
const DB_BBRAM    = 'bbram';
const DB_BOOTROMS = 'bootroms';

const DB_VERSION  = 1;

function promisifyRequest(request) {
    return new Promise((resolve, reject) => {
        request.onsuccess = () => resolve(request.result);
        request.onerror = () => reject(request.error);
    });
}

function openDb() {
    const request = indexedDB.open('gameboy-library', DB_VERSION);
    request.onupgradeneeded = () => {
        const db = request.result;
        db.createObjectStore(DB_LIBRARY,  { keyPath: 'crc32' });
        db.createObjectStore(DB_ROMS,     { keyPath: 'crc32' });
        db.createObjectStore(DB_BBRAM,    { keyPath: 'crc32' });
        db.createObjectStore(DB_BOOTROMS, { keyPath: 'id'    });
    };
    return promisifyRequest(request);
}

async function putRecord(storeName, value) {
    const db = await openDb();
    const tx = db.transaction(storeName, 'readwrite');
    await promisifyRequest(tx.objectStore(storeName).put(value));
}

async function getRecord(storeName, key) {
    const db = await openDb();
    const tx = db.transaction(storeName, 'readonly');
    return promisifyRequest(tx.objectStore(storeName).get(key));
}

async function getAllRecords(storeName) {
    const db = await openDb();
    const tx = db.transaction(storeName, 'readonly');
    return promisifyRequest(tx.objectStore(storeName).getAll());
}

async function hasRecord(storeName, key) {
    const db = await openDb();
    const tx = db.transaction(storeName, 'readonly');
    const count = await promisifyRequest(tx.objectStore(storeName).count(key));
    return count > 0;
}

/* Exported Functions: */

export async function loadLib() {
    return await getAllRecords(DB_LIBRARY);
}

export async function saveLibEntry(crc32, header, romSize, bbramSize) {
    await putRecord(DB_LIBRARY, {
        crc32,
        title: header.title,
        header,
        romSize,
        bbramSize,
        dateAdded: Date.now(),
        dateLastPlayed: Date.now(),
        dbVersion: DB_VERSION
    });
}

export async function loadLibEntry(crc32) {
    return await getRecord(DB_LIBRARY, crc32);
}

export async function hasLibEntry(crc32) {
    return await hasRecord(DB_LIBRARY, crc32);
}

export async function updateLibEntryLastPlayed(crc32) {
    const db = await openDb();
    const tx = db.transaction(DB_LIBRARY, 'readwrite');
    const st = tx.objectStore(DB_LIBRARY);
    const record = await promisifyRequest(st.get(crc32));
    if (record) {
        record.dateLastPlayed = Date.now();
        await promisifyRequest(st.put(record));
    }
}

export async function saveRomData(crc32, romBytes) {
    await putRecord(DB_ROMS, { crc32, data: romBytes });
}

export async function loadRomData(crc32) {
    return (await getRecord(DB_ROMS, crc32))?.data;
}

export async function hasRomData(crc32) {
    return await hasRecord(DB_ROMS, crc32);
}

export async function saveBbramData(crc32, ramBytes) {
    await putRecord(DB_BBRAM, { crc32, data: ramBytes });
}

export async function loadBbramData(crc32) {
    return (await getRecord(DB_BBRAM, crc32))?.data;
}

export async function hasBbramData(crc32) {
    return await hasRecord(DB_BBRAM, crc32);
}
