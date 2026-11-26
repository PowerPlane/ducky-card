# Codex review: V0.3_NFC_IE8T3

Two independent, read-only passes by Codex (gpt-6.2-astra, extra-high effort) on 2026-11-25. Links to Codex's scratch evidence files are removed; the figures are as it reported them.

## Pass 1: full review of the reworked corner

Covers the chip moved 0.4 mm inward, the 45-degree corners and the pad-terminated rail. Two things changed after this pass: antenna feed 2 went back beside feed 1 (pass 2), and the LED8 gap is recorded as 0.252 mm.

**I found no new routing or production-file blocker in variant A. I would hold assembly release for the microphone port.**

### Blocker

- **Both variants specify a 0.400 mm nonplated microphone port** at drill coordinates `(190.450, −74.778)`. JLCPCB publishes a **0.50 mm minimum**. Obtain CAM acceptance that the port will remain open and nonplated. The earlier assembly review also flagged U1/U3 lands. Final CAM disposition, live placement preview and parts allocation remain unverified. A production ZIP (`hardware/V0.3_NFC_IE8T3/production/V0.3_NFC_IE8T3-Gerbers.zip`) · [JLC capabilities](https://jlcpcb.com/capabilities/Capabilities?type=1)

### Should-fix

- **Correct the LED8 clearance figure.** Its diagonal clears Q9’s rectangular ground pad by **0.252 mm**, measured to the pad corner at `(185.900, 76.762655)`. The claimed 0.334 mm overstates the gap. The other two measurements reproduce: mic–LED8 **0.262 mm**, mic–U2.8 **0.303 mm**. All pass 0.150 mm. LED8 leaves at `x=185.2173` and enters Q9.1 horizontally at `y=76.3127`.
- **Verify powered operation before volume.** PC1 feeds U2 and the microphone through 470 Ω. The checked-in Pond firmware lists PC1 as unused. Verify rail voltage, microphone response and powered NFC writes on a pilot. · Firmware pin map (`firmware/production-pond/src/pins.h:73`)

### Notes

- **Topology.** No bare-track T or bare-track 90° bend on the specified routes. The rail joins the C1-bound trace at `(187.725,78.225)`, **0.050 mm inside U2.8**. One inherited 90° turn remains inside C1.1 at `(192.600,76.262655)`, on a 0.050 mm stub. It is cosmetic. The GPO via also changes heading 90° across layers in plan view.
- **Owner-save comparison.** U2 moved 0.400 mm inward to `(189.9,79.2)`. The rail stays at `y=81.05`. SCL is one straight segment from `y=80.360574` to `80.360600`. No unrelated track or via moved. **Four vias moved**, with 87 total: GPO `(185.5,79.3477)` and grounds `(183.8,79.3477)`, `(189.99,77.12)`, `(192.7625,81.05)`. The saved pour also changed outside the corner. At `x=175`, refilling the owner’s save reproduces the new boundary at `y=81.3005`.
- **Checks.** KiCad 10.0.8 `pcb drc --schematic-parity --all-track-errors` returned **14 baseline-identical J1 findings, zero unconnected, zero parity**. Refill reproduced A byte-for-byte. ERC’s **5 errors and 105 warnings** match baseline. U2 resolves from `Ducky_NFC:ST25DV04KC-IE8T3`, pins 1–8, with no library mismatch or missing-library finding.
- **Corner geometry.** Minimum routed gap is **0.185 mm**, SCL to U2.5. Pour clearance reaches **0.1505 mm**. Pin 4’s **0.875 mm** stub and via join its bottom copper island to the continuous front plane. U2 lands clear the edge by **1.400 mm**, the nearest feed by **0.510 mm**. U2 silk clears pads by **0.200 mm** and the moved hole by **0.260 mm**. Inherited bottom artwork grazes J1’s hole at `(192.015,84.251)`.
- **Antenna.** J1 is byte-identical to baseline. Feed lengths changed from **11.307/5.498** to **11.758/6.531 mm**. Length mismatch decreased, while feed-loop area, closed directly between terminal centers, grew from **7.05 to 12.64 mm²**. Feed 2, SCL and power pass under U2. Resonance and read range need physical measurement.
- **Production and B.** All **13** A fabrication files match fresh exports after timestamp normalization. BOM/CPL reproduce byte-for-byte, with all part codes verified and **40 bottom placements**. U2 is **C5267455, `(189.9, −79.2, 270°, bottom)`**. Both PCB revisions are `0-3`. B’s sole PCB edit since its previous save is the revision. Its copper, mask, paste, silk, outline and drills match baseline.

## Pass 2: re-check of the feed 2 reroute

**No blocker or should-fix finding. I found nothing wrong with the feed 2 reroute or its production exports.**

- **DRC.** KiCad 10.0.8 with `--schematic-parity --all-track-errors` returned the same 14 J1 antenna-footprint violations as the previous review, with **zero unconnected items and zero schematic parity issues**.
- **Route and clearance.** Feed 2 exits U2.3 to the right, clears the package body, runs at x = 193.79 mm beside feed 1, and follows the stated rejoin points to J1.2. Edge-to-edge gaps are **0.190 mm to U2.4**, **0.300 mm to feed 1**, and approximately **0.250 mm to J1 copper artwork**. The minimum is **0.1505 mm to the ground pour**, passing the 0.150 mm rule. Feed 2 measures **5.727 mm**, versus 5.498 mm baseline and 6.531 mm previously. Closing the feed centerlines between terminals gives **7.45 mm²**, versus 7.05 mm² baseline and 12.64 mm² previously.
- **Topology note.** Feed 2 has four 45° bends and no branch. The checked corner routes have no bare-track T or 90° bend. The inherited 90° turn lies inside C1.1, and the power branch lies inside U2.8.
- **Board and production.** No other footprint, track or via moved. U2 remains at **(189.9, 79.2) mm**, with **87 vias** board-wide. Twenty-two other routing objects received new UUIDs. A fresh refill reproduced the saved PCB byte-for-byte. All **13 Gerber, drill and drill-map files** matched fresh exports after timestamp normalization. BOM and CPL matched byte-for-byte.

## Pass 3: the rail routed to capacitor C1 first (the board as delivered)

After pass 2 the owner chose a cleaner pin 8: the rail now ends on C1's pad and pin 8 keeps one track to C1. In this report "A" is this board and "B" is the DFN board in `hardware/V0.3_NFC_IER6C3/`. Passes 1 and 2 above describe the earlier joint on pin 8's pad.

**Blocker — none found for a pilot order of either variant.** A’s reroute passes the 0.150 mm rule. B’s **0° bottom-side U2 placement is correct**. Live JLC placement approval, parts allocation, and physical RF performance remain unverified.

**Should-fix before volume**

- **A — verify powered writes and read range on a pilot.** Feeding the 100 nF C1 pad before U2.8 is sound. The capacitor-to-pin trace is 6.35 mm. The reroute splits the bottom ground pour between C1.2 and U2.4, but both regions connect to the continuous front plane. Pin 4 has a 0.875 mm trace to its via at `(192.7625, 81.05)`. A fresh refill reproduced the saved board byte-for-byte.
- **B — label the archived placement review.** It still gives 270° for the previous C2654866. I fitted all nine numbered lands from a cached JLC footprint capture against the mirrored board coordinates. C2908287 fits at 0° with **0.093 mm maximum center error**. Board pin 1 is at `(192.990, 78.550155)` and misses by **3.273 mm** at 180°. The old part fits at 270°. · Archived review (`hardware/V0.3_NFC_IER6C3/review/codex-review.md:21`)

**Notes**

- **A routing.** Exactly four rail segments changed. All 46 footprints and 87 vias are identical to prior A. The rail follows `(190.9,81.05) → (191.3,80.65) → (191.3,78.2127) → C1.1 (193,76.5127)`. U2.8 has one unchanged exit through `(190.6877,78.225)` to C1.1 `(192.6,76.3127)`. No bare-track T or 90° bend. The inherited 90° turn is inside C1.1. Board tracks (`hardware/V0.3_NFC_IE8T3/V0.3_NFC_IE8T3.kicad_pcb:77246`)
- **A clearances.** New rail to SCL measures **0.190 mm**, to rounded U2.1 **0.321 mm** (bounding rectangle **0.280**), and to U2.7 **1.875 mm**. The claimed **0.350 mm** is from the unchanged pin-8 trace. C1’s parallel arrivals are **0.424 mm center-to-center, 0.224 mm edge-to-edge**. Both terminate inside the pad. I found no copper sliver, consequential acid trap, or issue with the masked power and SCL traces under the body. J1 and both feeds are unchanged. The changed rail clears each feed trace by at least **1.263 mm**.
- **Checks and exports.** KiCad 10.0.8 `drc --schematic-parity --all-track-errors` returned identical results for A, B and baseline — **14 J1 violations, zero unconnected, zero parity**. ERC also matched baseline at **5 errors, 105 warnings**. Both BOM/CPL pairs reproduced byte-for-byte, with 40 bottom placements each. All 13 fabrication files matched fresh exports after timestamp normalization. B’s copper, mask, paste, silk, outline and drills also matched baseline after project-header normalization.
- **B substitution.** Beyond the rename and U2 value, LCSC and offset edits, ten hidden text fields gained cosmetic stroke widths. Both codes share the 2 × 3 mm UFDFPN8, pinout and 28.5 pF tuning capacitance. The upper temperature rating drops from **105°C to 85°C**. JLC’s Nov 24 snapshot showed 237 available for C2908287 and zero for C2654866. [ST ordering table 261](https://www.st.com/resource/en/datasheet/st25dv04k.pdf)
