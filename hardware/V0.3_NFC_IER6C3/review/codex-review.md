# Codex review: V0.3_NFC_IER6C3

Two independent, read-only passes by Codex (gpt-6.2-astra, extra-high effort) on 2026-11-25. Links to Codex's scratch evidence files are removed. "B" is this board; "A" is the TSSOP board in `hardware/V0.3_NFC_IE8T3/`.

This folder was `V0.3_NFC_IER8C3` until the chip changed to ST25DV04K-IER6C3 (LCSC C2908287), which is in stock. The chip is now placed at **0 degrees**; pass 2 checks that.

## Pass 1: before the chip swap

Written when this board carried ST25DV04K-IER8C3 (C2654866). Its 270-degree rotation and its stock finding apply to that part, not to the current one.

**I found no new PCB routing blocker in either variant.** A is the better pilot candidate. Hold assembly release until JLC confirms the microphone port and U2 placement. B also needs a parts allocation.

### Blocker

- **Both — microphone port.** Both production `*-NPTH.drl` files specify a **0.400 mm** nonplated hole at **(190.450, 74.778)**, below JLC’s published **0.50 mm** minimum. Obtain CAM acceptance that the acoustic port will remain open and nonplated. The handover also records 36 assembly “Danger” findings on U1/U2/U3. U1 and U3 retain those lands on both boards. Neither new assembly has a final CAM disposition. [JLC capabilities](https://jlcpcb.com/capabilities/Capabilities?type=1)
- **B — U2 supply.** JLC lists **zero stock** for C2654866. This is the **K** device. The previously assembled boards used **KC**, C5221756. Confirm allocation and K firmware behavior before committing the lot. [JLC part listing](https://jlcpcb.com/partdetail/STMicroelectronics-ST25DV04KIER8C3/C2654866)

### Should-fix / verify before volume

- **Both — powered operation.** PC1 supplies `/MIC_NFC_PWR` through R1, 470 Ω. The current Pond firmware lists PC1 as unused. Verify rail voltage and powered NFC writes on a pilot. ERC returned five errors on each board, all inherited. A adds one warning because its edited eight-pin symbol still uses the DFN library ID. Updating it from that library can restore pin 9. Firmware pin map (`firmware/production-pond/src/pins.h:73`) · A schematic (`hardware/V0.3_NFC_IE8T3/V0.3_NFC_IE8T3.kicad_sch:6748`)
- **A — antenna and decoupling.** J1 is byte-identical. Its two feed lengths changed from **11.307 / 5.498 mm** to **11.416 / 6.516 mm**. Length mismatch decreased, but feed 2 now runs under U2 and separates farther from feed 1. Changed loop area and stray capacitance warrant a pilot resonance and battery-out read-range check. C1’s corresponding pad centers are **5.52 mm from pin 8** and **5.84 mm from pin 4**. The supply trace to C1 is **5.95 mm**. A closer capacitor would shorten that loop.

### Notes

- **DRC and fabrication files.** KiCad 10.0.8 `pcb drc --schematic-parity --all-track-errors` returned the same **14 J1 errors at identical coordinates, zero unconnected items and zero parity findings** on all three boards, including after refill. Both production ZIPs matched fresh Toolkit exports across all 13 files after ignoring date/project headers. B’s copper, mask, paste, silk, outline and drills match baseline.
- **A — routing.** All eight named nets changed. Vias fell **91→87**. The closest rerouted trace is `/MIC_ANALOG` to U2.8 at **0.155 mm**, passing the **0.150 mm** rule and JLC’s **0.10 mm** 1-oz pad-to-track limit. No acute track bends. SCL, power and feed 2 run under the molded body. Right-hand lands clear the edge by **1.00 mm** and footprint silk clears pads by **0.20 mm**. Refill reproduced the saved copper exactly. Pin 4’s bottom ground island connects through **1.01 mm** of trace to the continuous front plane at **(192.797, 81.036)**. Outside the corner, rounded long-track endpoints caused minute copper differences.
- **Assembly and orientation.** Both BOMs and CPLs reproduce exactly: **40 bottom placements**, SW1 omitted, LED21–22 C965861, LED23–24 C965860, nine MOSFETs at 270°, U1 at 45°. B’s CPL is byte-identical to the hand correction. U2 CPL coordinates are A **(190.300, −79.200)** and B **(191.490, −79.300155)**. A’s 4.4 × 3 mm, 0.65 mm-pitch TSSOP matches ST’s package and back-side pin numbering. **270° is supported for both** by supplier-footprint IDs and published numbered lands. Maximum pad-center differences were **0.065 mm A / 0.100 mm B**. The live JLC placement preview and physical RF performance remain unverified. Title-block `0-2` is cosmetic. [ST package drawing](https://www.st.com/resource/en/datasheet/st25dv04kc.pdf)

## Pass 2: after the swap to C2908287

**Blocker — none found for a pilot order of either variant.** A’s reroute passes the 0.150 mm rule. B’s **0° bottom-side U2 placement is correct**. Live JLC placement approval, parts allocation, and physical RF performance remain unverified.

**Should-fix before volume**

- **A — verify powered writes and read range on a pilot.** Feeding the 100 nF C1 pad before U2.8 is sound. The capacitor-to-pin trace is 6.35 mm. The reroute splits the bottom ground pour between C1.2 and U2.4, but both regions connect to the continuous front plane. Pin 4 has a 0.875 mm trace to its via at `(192.7625, 81.05)`. A fresh refill reproduced the saved board byte-for-byte.
- **B — label the archived placement review.** It still gives 270° for the previous C2654866. I fitted all nine numbered lands from a cached JLC footprint capture against the mirrored board coordinates. C2908287 fits at 0° with **0.093 mm maximum center error**. Board pin 1 is at `(192.990, 78.550155)` and misses by **3.273 mm** at 180°. The old part fits at 270°. · Archived review (`hardware/V0.3_NFC_IER6C3/review/codex-review.md:21`)

**Notes**

- **A routing.** Exactly four rail segments changed. All 46 footprints and 87 vias are identical to prior A. The rail follows `(190.9,81.05) → (191.3,80.65) → (191.3,78.2127) → C1.1 (193,76.5127)`. U2.8 has one unchanged exit through `(190.6877,78.225)` to C1.1 `(192.6,76.3127)`. No bare-track T or 90° bend. The inherited 90° turn is inside C1.1. Board tracks (`hardware/V0.3_NFC_IE8T3/V0.3_NFC_IE8T3.kicad_pcb:77246`)
- **A clearances.** New rail to SCL measures **0.190 mm**, to rounded U2.1 **0.321 mm** (bounding rectangle **0.280**), and to U2.7 **1.875 mm**. The claimed **0.350 mm** is from the unchanged pin-8 trace. C1’s parallel arrivals are **0.424 mm center-to-center, 0.224 mm edge-to-edge**. Both terminate inside the pad. I found no copper sliver, consequential acid trap, or issue with the masked power and SCL traces under the body. J1 and both feeds are unchanged. The changed rail clears each feed trace by at least **1.263 mm**.
- **Checks and exports.** KiCad 10.0.8 `drc --schematic-parity --all-track-errors` returned identical results for A, B and baseline — **14 J1 violations, zero unconnected, zero parity**. ERC also matched baseline at **5 errors, 105 warnings**. Both BOM/CPL pairs reproduced byte-for-byte, with 40 bottom placements each. All 13 fabrication files matched fresh exports after timestamp normalization. B’s copper, mask, paste, silk, outline and drills also matched baseline after project-header normalization.
- **B substitution.** Beyond the rename and U2 value, LCSC and offset edits, ten hidden text fields gained cosmetic stroke widths. Both codes share the 2 × 3 mm UFDFPN8, pinout and 28.5 pF tuning capacitance. The upper temperature rating drops from **105°C to 85°C**. JLC’s Nov 24 snapshot showed 237 available for C2908287 and zero for C2654866. [ST ordering table 261](https://www.st.com/resource/en/datasheet/st25dv04k.pdf)
