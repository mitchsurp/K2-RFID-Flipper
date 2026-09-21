# Creality CFS RFID for Flipper Zero

A native Flipper Zero application port of [DnG-Crafts/K2-RFID](https://github.com/DnG-Crafts/K2-RFID) for reading, writing, editing, and emulating RFID filament spool tags used by the **Creality Filament System (CFS)** on the **Creality K2, K2 Plus, K1 Series (with CFS upgrade), and HI printers**.

---

## Features

- **Scan & Decrypt Spool Tags**: Place any Creality CFS spool tag on the back of the Flipper Zero. The app automatically derives the sector keys from the tag's UID, reads Sector 1 & Sector 2, decrypts the payload with AES-128, and displays:
  - Filament Brand & Material Name (e.g. *Creality Hyper PLA*)
  - Material Type (e.g. *PLA, PETG, ABS, TPU, PLA-CF*)
  - Color (Hex code + closest matched human-readable color name)
  - Spool Size & Length (e.g. *1 KG / 330m, 750g, 500g, 250g*)
  - Serial Number, Batch Code, and Production Date
  - Target Printer Model (*K2, K1, HI*)
  - 4-byte Tag UID
- **Write Spool Tags**: Program blank physical **MIFARE Classic 1k** tags with encrypted CFS spool data and write trailer block 7 with the cryptographic key derived from the tag's UID.
- **Real-Time Spool Emulation**: Emulate any configured or scanned spool directly from the Flipper Zero via NFC. Hold the Flipper against the CFS spool reader slot to supply filament spool identity without needing physical RFID tags!
- **Save to SD Card (.nfc)**: Save any spool configuration as a standard Flipper `.nfc` file into `/ext/nfc/CFS/` for instant emulation directly from the Flipper's native NFC application or this app.
- **Load & Inspect Saved Spools**: Built-in file browser to load, inspect, edit, or emulate any `.nfc` file stored on the SD card.
- **Format / Erase Tags**: Wipe CFS sectors and restore Sector 1 & Sector 2 back to factory default transport keys (`FF FF FF FF FF FF`).
- **Complete Material Database**: Built-in offline database of all **66 official Creality filaments** (Hyper PLA, Hyper PETG, Hyper ABS, CR-Silk, eSUN PLA+, Polymaker PolySonic, etc.) with recommended nozzle and bed temperatures.
- **Desktop Companion CLI**: Includes [`tools/cfs_spool_tool.py`](file:///Users/mitch/Desktop/k2rfid/tools/cfs_spool_tool.py) to generate and decode Flipper `.nfc` files directly on PC/Mac/Linux.

---

## Tag Format & Cryptography

Creality CFS uses **MIFARE Classic 1K** RFID tags:

| Sector | Blocks | Content | Key / Auth |
|---|---|---|---|
| **0** | 0..2 | Manufacturer block (UID, SAK `0x08`, ATQA `0x04 0x00`) | Key A: `FF FF FF FF FF FF` |
| **1** | 4..6 | 48 bytes payload encrypted with **AES-128-ECB** using `d_key` | Key A/B: **Derived Key** from UID |
| **1 Trailer** | 7 | Key A (`encKey`), Access Bits `FF 07 80 69`, Key B (`encKey`) | Key A/B: **Derived Key** |
| **2** | 8..10 | 48 bytes plaintext ASCII with Printer Model (`"K2"`, `"K1"`, `"HI"`) | Key A: `FF FF FF FF FF FF` |
| **2 Trailer** | 11 | Key A (`FF..FF`), Access Bits `FF 07 80 69`, Key B (`FF..FF`) | Key A: `FF FF FF FF FF FF` |
| **3..15** | 12..63 | Unused / Blank | Key A: `FF FF FF FF FF FF` |

### Keys

- **Key Derivation Key (`u_key`)**:
  - ASCII: `q3bu^t1nqfZ(pf$1`
  - Hex: `71 33 62 75 5E 74 31 6E 71 66 5A 28 70 66 24 31`
  - Derivation: The 4-byte UID is repeated 4 times (16 bytes), encrypted with AES-128-ECB using `u_key`. The first 6 bytes of the ciphertext become Sector 1 Key A and Key B.
- **Data Encryption Key (`d_key`)**:
  - ASCII: `H@CFkRnz@KAtBJp2`
  - Hex: `48 40 43 46 6B 52 6E 7A 40 4B 41 74 42 4A 70 32`
  - Payload Encryption: Blocks 4, 5, and 6 (48 bytes total) are encrypted with AES-128-ECB using `d_key`.

### Payload Layout (Sector 1, 48 bytes)

```
 AB124 0276 A2 1 01001 0 FFFFFF 0330 000001 00000000000000
| date|vend|bt| |matId| | color| len|serial|    reserve   |
```

- `0..4` (5 chars): Date code (default: `"AB124"`)
- `5..8` (4 chars): Vendor ID (`"0276"` = Creality)
- `9..10` (2 chars): Batch code (`"A2"`)
- `11..16` (6 chars): Filament ID (`'1'` + 5-digit Material ID, e.g. `"101001"` for Hyper PLA)
- `17..23` (7 chars): Color code (`'0'` + 6-digit RGB hex, e.g. `"0FFFFFF"` for White)
- `24..27` (4 chars): Length code (`"0330"` = 1 KG, `"0247"` = 750 G, `"0198"` = 600 G, `"0165"` = 500 G, `"0082"` = 250 G)
- `28..33` (6 chars): Serial number (e.g. `"000001"`)
- `34..47` (14 chars): Reserved padding (`"00000000000000"`)

---

## Installation

### Method 1: Pre-built FAP
Copy [`dist/k2_rfid.fap`](file:///Users/mitch/Desktop/k2rfid/dist/k2_rfid.fap) to your Flipper Zero's SD card under `apps/NFC/`.

### Method 2: Build from Source with uFBT
```bash
# Clone the repository
cd k2rfid

# Build the application
ufbt

# Upload and launch directly on Flipper Zero connected via USB
ufbt launch
```

---

## Desktop Companion Tool

A python utility is provided in `tools/cfs_spool_tool.py` to create and decode Flipper `.nfc` spool files on your computer:

```bash
# Generate a 1 KG White Hyper PLA spool file for K2
python3 tools/cfs_spool_tool.py generate -m 01001 -c FFFFFF -w "1 KG" -p K2 -o Hyper_PLA_White.nfc

# Decode and inspect any .nfc spool file
python3 tools/cfs_spool_tool.py decode Hyper_PLA_White.nfc

# List available materials
python3 tools/cfs_spool_tool.py list
```

---

## Project Structure

```
k2rfid/
├── application.fam         # Flipper Application Manifest
├── k2_rfid_app.h           # Main application architecture & state
├── k2_rfid_app.c           # Entry point, GUI & dispatcher lifecycle
├── k2_rfid.png             # 10x10 monochrome application icon
├── crypto/
│   ├── k2_crypto.h         # AES-128 ECB & key derivation interface
│   └── k2_crypto.c         # Self-contained zero-dependency AES implementation
├── data/
│   ├── k2_database.h       # Filament materials, colors, weights, payload codecs
│   └── k2_database.c       # 66-material database & color matcher
├── nfc/
│   ├── k2_worker.h         # Background NFC poller, writer, & emulator worker
│   └── k2_worker.c         # Mifare Classic 1K poller/listener operations
├── scenes/                 # Scene manager UI screens
│   ├── k2_scene.h          # Scene manager declarations
│   ├── k2_scene.c          # Scene handlers dispatch table
│   ├── k2_scene_main_menu.c# Main menu
│   ├── k2_scene_scan.c     # Scan physical tag
│   ├── k2_scene_tag_info.c # Spool details & action buttons (Save/Emulate/Edit)
│   ├── k2_scene_write.c    # Write physical tag
│   ├── k2_scene_emulate.c  # Real-time NFC spool emulation
│   ├── k2_scene_config.c   # Spool settings editor (Printer, Material, Color, Size)
│   ├── k2_scene_save.c     # Save .nfc file to SD card
│   ├── k2_scene_saved_spools.c # File browser for saved .nfc files
│   ├── k2_scene_format.c   # Wipe and restore tag to factory transport keys
│   └── k2_scene_about.c    # Credits and version info
├── tools/
│   └── cfs_spool_tool.py   # Desktop python companion generator/decoder
└── dist/
    └── k2_rfid.fap         # Compiled Flipper Zero binary
```

---

## Credits & References

- Ported from [DnG-Crafts/K2-RFID](https://github.com/DnG-Crafts/K2-RFID)
- Compatible with Creality K2 Plus, K1, K1 Max, K1C, and HI with Creality Filament System (CFS).
