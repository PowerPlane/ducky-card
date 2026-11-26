# JLCDFM results: V0.3_NFC_IER6C3

Run on 2026-11-25 on the production files in `../production/` (Gerbers, BOM, CPL), after the tag changed to ST25DV04K-IER6C3. The viewer link only opens in the browser session that uploaded it: https://jlcdfm.com/viewer?pcbUploadFileId=648625630147510273

| Check | Result |
| --- | --- |
| Bare board, Danger | 22: silkscreen to pad 2, silkscreen to hole 20. All inherited artwork |
| Bare board, Warning | 359 |
| BOM match | 10 of 10 lines; U2 matched to ST25DV04K-IER6C3, C2908287 |
| Assembly, Danger | 36: U1 (MCU) 21, U3 (mic) 6, U2 (NFC tag) 9 |
| Pins without a pad | 0 |
| U2 orientation | Placed at 0 degrees. In `jlcdfm-U2-placement.jpg` (seen from the back) the chip's pin 1 dot sits at the same corner as the board's pin 1 mark |

The nine U2 findings are the DFN's short lands (pin inner edge 8, lead area over pad 1), the same count this board had with the previous chip at 270 degrees. The U1 and U3 findings are on the same lands V0.2 was built with.
