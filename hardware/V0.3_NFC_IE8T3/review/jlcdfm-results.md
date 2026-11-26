# JLCDFM results: V0.3_NFC_IE8T3

Run on 2026-11-25 on the production files in `../production/` (Gerbers, BOM, CPL), after the rail was routed to capacitor C1 first. The viewer link only opens in the browser session that uploaded it: https://jlcdfm.com/viewer?pcbUploadFileId=648623606101696514

| Check | Result |
| --- | --- |
| Bare board, Danger | 21: silkscreen to pad 2, silkscreen to hole 19. All inherited artwork |
| Bare board, Warning | 342. One more than the previous revision: the ground pour is 0.15 mm from one more of U2's pads, the same warning 41 other pads carry |
| BOM match | 10 of 10 lines; U2 matched to ST25DV04KC-IE8T3, C5267455 |
| Assembly, Danger | 27: U1 (MCU) 21, U3 (mic) 6, U2 (NFC tag) 0 |
| Pins without a pad | 0 |

The U1 and U3 findings are on the same lands V0.2 was built with. `jlcdfm-U2-placement.jpg` was taken on the previous revision; U2 has not moved since.
