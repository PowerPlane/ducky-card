# The board

The Ducky business card (`business-card-v1`): a 2-layer PCB cut to standard business-card size with a CR2032 holder, a 24-LED matrix, a MEMS microphone, and an NFC tag.

## Highlights

| | |
|---|---|
| **Form factor** | V0.3 board outline measures 88.9 × 50.8 mm |
| **MCU** | ATtiny1616-M (Microchip, AVR tinyAVR 1-series, 16 kB flash, 2 kB SRAM, VQFN-20) |
| **Power** | CR2032 coin cell (the "coin" you insert to activate the card) |
| **Display** | 24 side-view LEDs in 9 switched cathode banks |
| **Audio in** | LMA2718B381-OA7 analog MEMS microphone |
| **NFC** | ST25DV-series dynamic tag with 4 Kb EEPROM. V0.3 specifies ST25DV04KC-IE8C3 |
| **Programming** | UPDI (single-wire) via SerialUPDI |

## Status

- **Hardware**: V0-1 and V0-2 fabbed (KiCad 10); V0-3 is a routed prototype with switched mic/NFC power and a front touch electrode. See the [V0.3 review](../hardware/Ducky-Business-Card-V0-3/REVIEW.md) before assembly. See the table below.
- **Firmware**: bring-up and production environments in [`firmware/`](../firmware/).
- **Enclosure**: see [`enclosure/`](../enclosure/).

## Hardware files

Each board revision has its own folder under [`hardware/`](../hardware/). Open the `.kicad_pro` inside one in KiCad to view its schematic and PCB.

| Revision | Folder | Project file | Fab outputs |
|---|---|---|---|
| V0-1 | [`Ducky Business Card V0-1/`](../hardware/Ducky%20Business%20Card%20V0-1/) | `Insert-Coin-Business-Card.kicad_pro` | yes |
| V0-2 | [`Ducky-Business-Card-V0-2/`](../hardware/Ducky-Business-Card-V0-2/) | `Ducky-Business-Card-V0-2.kicad_pro` | yes |
| V0-3 | [`Ducky-Business-Card-V0-3/`](../hardware/Ducky-Business-Card-V0-3/) | `Ducky-Business-Card-V0-3.kicad_pro` | [prototype exports](../hardware/Ducky-Business-Card-V0-3/production/) |

A revision's `production/` folder holds the BOM (`bom.csv`), pick-and-place positions (`positions.csv`), designator map, IPC netlist, and the fab zip. V0-1 and V0-2 share the same BOM and MCU pin nets, so the firmware's `pins.h` fits both.

V0-3 includes a project-local NFC antenna footprint with explicit electrical connections for its printed winding. Its duck touch and test-pad footprints come from kicad-libraries.
