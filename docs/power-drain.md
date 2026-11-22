# Battery drain: findings and measurements

November 2026. The question was why a CR2032 left in the card goes flat, and what the simplest fix is, so the cell can stay in the holder without being pulled after each show.

**Where it stands:**

- **Measured:** after a great-luck or little-luck show, the card stays awake at 3.0 to 3.7 mA with every LED off. After uncertain or bad luck it reads 0.0 mA on the same meter.
- **Likely cause (firmware):** the timed-sleep routine fails on its second call in a boot. A fix is on branch `claude/sweet-noether-g11mp1` but has not been run on a card.
- **Still unmeasured:** the standby draw of the mic and NFC tag, which sit on the battery net. The datasheets put it near 0.2 mA, which the meter cannot resolve.

Every number below is labelled with where it came from: **measured** on a card, read from a **datasheet**, **estimated** from those, or a **hypothesis**.

## Summary

| # | Issue | Basis | Effect on a CR2032 (about 220 mAh) | Status |
|---|---|---|---|---|
| 1 | Timed sleep stays awake on its second call | Measured; cause is a hypothesis | 3.0 to 3.7 mA: flat in about 2.5 to 3 days. Link not reset. | Fix pushed, untested |
| 2 | Mic and NFC tag always powered | Datasheet estimate | About 0.2 mA: flat in about 6.5 weeks | Unmeasured; board change planned |
| 3 | No LED current limiting | Measured current; rail sag unmeasured | 25.5 mA per lit 4-LED bank; MCU possibly out of spec while lit | Open; no resistor wanted |
| 4 | Copper under the cell's outer edge | Board files | None unless the mask wears through | Latent risk |
| 5 | Docs and comments describe parts no longer on the board | Repo history | Misleads future work | Not fixed |

## Measurements

### Setup

- **Date:** 2026-11-21.
- **Meter:** Pokit Meter (POK-TRK), DC current mode. Specified range 1 mA to 2 A, ±1% (retailer listings). The app shows 0.1 mA steps, but below 1 mA the reading is outside the specified range. Read 0.0 as "under about 1 mA", not zero.
- **Wiring:** meter in series with the ground return. Cell positive to the holder's clip, meter + lead on TP3 (board ground), meter − lead on the cell's negative face.
- **Firmware:** whatever the card already carried, without the fix. The variant was not recorded: `production-nfc` or `production-pond`, which share the timed-sleep code.
- **Board revision:** not recorded. V0-1, V0-2 and V0-3 are electrically identical (see Issue 2).

### Run 1: great luck, screen recording

Two-minute screen recording of the meter app, read at 2 frames per second for the first 30 s and every 4 s after that.

| Recording time | Reading | What the firmware is doing |
|---|---|---|
| 0:00 to 0:03.5 | 0.0 mA | Before power-up |
| 0:04 to 0:04.5 | 16.3 mA | Show starts (boot capture ramp) |
| 0:05 to 0:06 | 26.0 mA | Show animation (software PWM across banks) |
| 0:06.5 | 18.5 mA | Show animation |
| 0:07 to 0:09 | 26.7 mA | Show animation |
| 0:09.5 to 0:10 | **4.2 mA** | LEDs briefly off mid-show, CPU awake |
| 0:10.5 to 0:12.5 | 26.6 mA | Reveal blinks and ripple |
| 0:13 to 0:24 | **25.5 mA**, steady | 12 s great-luck hold: bank 4 (4 LEDs) latched on, CPU meant to be in timed sleep |
| 0:24.5 to 2:00 | **3.7 mA**, steady for 95 s | LEDs off. Should be in power-down sleep for the live-URL window. |

What the numbers imply:

- **Awake versus asleep:** 3.7 mA with the LEDs off is the CPU running. The ATtiny1616 draws 3.1 mA typ at 10 MHz and 3 V (datasheet Table 36-4); the mic, the tag and the running timers make up the rest. The 4.2 mA moment mid-show is the same awake state; the extra 0.5 mA fits the mic's ADC, which is still on before the reveal (325 µA typ, Table 36-7).
- **LED current:** one fully lit 4-LED bank draws about 25.5 mA, roughly 6.4 mA per LED. Nothing limits it except the cell, the LEDs and the FET.

### Run 2: all four fortunes

Reported by the tester; not recorded. Unfixed firmware.

| Fortune | Reading after the LEDs go off | Timed-sleep calls in this boot before that point |
|---|---|---|
| Great luck | 3.0 mA | 2 (12 s hold, then the live-URL wait) |
| Little luck | 3.2 mA | 2 (12 s hold, then the live-URL wait) |
| Uncertain luck | 0.0 mA | 1 (the live-URL wait) |
| Bad luck | 0.0 mA | 1 (the live-URL wait) |

The lower 3.0 to 3.2 mA against Run 1's 3.7 mA is consistent with the same awake state on a cell that had sagged during testing. How long each reading was watched was not recorded.

### What the measurements do not tell us

- **Sleep current below 1 mA:** mic, tag and MCU together. Datasheet estimate about 0.2 mA (Issue 2).
- **Rail voltage while LEDs are lit:** the rail-sag test was skipped (Issue 3).
- **Whether the 3 mA state ever ends:** the recording stopped 95 s after the hold. The live-URL window is 120 s in `production-nfc` and 300 s in `production-pond`.
- **Whether great or little luck ever reset the link:** a phone tap after the window would show it.

## Issue 1: the timed sleep stays awake on its second call

**Evidence.** The awake state follows exactly the fortunes that call `sleep_timed_seconds()` twice in one boot:

- **Great and little luck:** call it for the 12 s hold (`firmware/production-nfc/src/anim.cpp:117`, `:144`; `firmware/production-pond/src/anim.cpp:132`, `:159`), then again for the live-URL wait (`firmware/production-nfc/src/main.cpp:76`; `firmware/production-pond/src/main.cpp:305`).
- **Uncertain and bad luck:** call it only for the live-URL wait.

The first call works: the hold ends after 12 s and the LEDs go off on time.

**The code before the fix** (identical in `production-nfc` and `production-pond`):

```cpp
while (RTC.STATUS != 0) { /* wait for any pending sync */ }
RTC.CLKSEL     = RTC_CLKSEL_INT32K_gc;
RTC.PITCTRLA   = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;  // 1 Hz
RTC.PITINTCTRL = RTC_PI_bm;
// ... sleep until the tick count reaches zero ...
RTC.PITINTCTRL = 0;
RTC.PITCTRLA   = 0;   // stops the PIT on the way out
```

**Hypothesis for the mechanism.** The RTC itself is never enabled (millis runs on TCB0), so the PIT runs with the RTC disabled.

- **Microchip erratum 2.6.2,** "Disabling the RTC Stops the PIT," applies to ATtiny1614/1616/1617 silicon Rev. A and Rev. B (DS80000886C). Writing `PITEN` to 0 stops the RTC, and the workaround is to never disable either one while the other is in use.
- **megaTinyCore's errata notes** (`megaavr/extras/Errata.md`, RTC section) describe this sequence on 0/1-series parts: PIT enabled with the RTC disabled, then a `while (RTC.STATUS)` that never returns because `RTC_CNTBUSY` stays at 1.

That would leave the second call spinning in its first line, CPU awake with the LEDs off, forever. This specific hang comes from megaTinyCore's notes, not Microchip's document; the measurements only prove the CPU is awake after the second call.

**Why the other explanations don't fit:**

- **I2C hang:** no I2C happens between the LEDs going off and the 3.7 mA starting in `production-nfc`. In `production-pond` the I2C path before the wait runs only if the fortune patch failed, which would hit all four fortunes alike.
- **Interrupts keeping the CPU awake:** in power-down, no enabled interrupt besides the PIT can keep waking it.
- **The plain `production` variant:** it calls the routine only once per boot, so it should not show this at all.

**Impact.** About 3 to 3.7 mA until the cell is flat, roughly 2.5 to 3 days. The live-URL window never ends, so the fortune digit (and in `production-pond`, an armed claim) is never reset on the tag.

**Fix.** Commits `005a1c0` (`production-pond`) and `2ff3da7` (`production-nfc`, kept separate because `platformio.ini` calls that variant frozen).

- **PIT handling:** started once and left counting, following Microchip's workaround. Nothing waits on `RTC.STATUS`.
- **Interrupt masking:** each call only unmasks the PIT interrupt, clearing any stale flag first, and masks it again on exit.
- **Stopping the PIT:** happens only in `sleep_forever()`, where nothing needs it again.
- **Side effect:** a second or later call can return up to 1 s early, because the PIT free-runs between calls.

**Not yet verified on hardware.** The fix was syntax-checked with `avr-g++ -fsyntax-only` against megaTinyCore's ATtiny1616 headers. There was no full PlatformIO build, because PlatformIO's registry was unreachable from the environment where it was written.

**How to confirm.** Flash the branch with great luck forced and repeat Run 1. The reading should fall to 0.0 mA when the hold ends.

```bash
cd firmware && PLATFORMIO_BUILD_FLAGS="-DFORCE_FORTUNE=0" pio run -e production-pond -t upload
```

## Issue 2: the mic and NFC tag are always powered

**Board.** U2 (ST25DV04K-IER8C3) pin 8 and U3 (LMA2718B381-OA7) pin 3 sit on the VCC net with the cell's positive terminal. There is no switch.

- **Revisions:** extracting nets from the KiCad files gives identical connectivity for V0-1, V0-2 and V0-3. V0-3 differs from V0-2 only in its name.
- **Spare MCU pins:** PB5, PC1, PC2 and PC3 are unconnected; PB4 has a stub net with nothing at the other end.
- **Spare tag pins:** GPO (pin 7) and V_EH (pin 1) are unconnected.

**Datasheet numbers:**

| Part | Standby current | Source |
|---|---|---|
| ST25DV04K, VCC on, no RF, I2C idle | 76 µA typ, 100 µA max at 3.3 V | DS10925 Rev 9, Table 246, ICC1_PON |
| ST25DV04K low-power pin | Not available: LPD exists only on the 10-ball and 12-pin packages | DS10925 §2.2.2 |
| LMA2718B381 mic | About 120 µA | Listings for sibling variants (OAK03, OFA18); the OA7 datasheet was not readable from here |
| ATtiny1616, power-down | 0.1 µA typ, 2 µA max at 25 °C, 3 V | DS40002204A, Table 36-5 |

**Estimate.** About 0.2 mA continuously, so a fresh cell lasts about 6.5 weeks with no use. Not measured: the meter's floor is 1 mA.

**Measuring it with the same meter.** Put a 1 kΩ resistor in the ground lead with a jumper across it. Run a show that ends in uncertain or bad luck, or any fortune on the fixed build, with the jumper in place. Wait 6 minutes after the LEDs go off, remove the jumper, and read the voltage across the resistor in voltage mode: 1 mV is 1 µA. The meter's 10 mV voltage floor means this resolves down to 10 µA. Keep the jumper on at all other times, because the card browns out through the resistor when the MCU wakes.

**Board plan if the measurement justifies it (V0-3), no added parts:**

- **Switched supply:** move U2 VCC, U3 VDD and C1 from VCC to a new net driven by PC1 (pin 16).
- **Wake line:** route U2 GPO (pin 7) to PC2 (pin 17) so a phone tap can wake the card.
- **Firmware:**
  - Drive PC1 high at boot and before any I2C.
  - Wait at least the tag's 0.6 ms boot time; the mic's start-up time is unknown.
  - Cut PC1 only after the last EEPROM write has finished (`NDEF_EEPROM_WRITE_MS` after it), and disable the PB0/PB1 pull-ups first.
- **Fallback:** a three-pad solder jumper selecting VCC or PC1 for that net would let a board go back without a respin.

**Open questions on this plan:**

- **VCC rise rate.** The tag requires VCC to rise no faster than 1 V/µs (DS10925 §3.1, "Power-up conditions").
  - The ATtiny datasheet does not specify how much current a pin delivers into a capacitor. The 40 mA in Table 36-1 is an absolute maximum rating, not a limit on the driver.
  - Into C1's 100 nF, the rise is estimated at 0.2 to 1 V/µs, close to the limit. An earlier note in this investigation called this clearly safe; that was wrong.
  - On ST's forum, a GPIO into 10 nF measured about 2 V/µs, and ST pointed to the datasheet's 100 nF as the fix.
  - Options: a firmware soft start (pulse the pin, or pre-charge C1 through the internal pull-up before driving it), or one 47 to 100 Ω series resistor.
- **Main-rail dip at switch-on.** Charging C1 briefly pulls current from the cell, of the same order as lighting one LED bank (about 25 mA), which the show already does many times.
- **Cutting the tag's supply while a phone's field is present.**
  - ST's errata for the newer KC variant (ES0617) describe a lock-up when VCC is pulled to 0 and back with RF present. Whether ES0616 lists the same for this part (ST25DV04K, not KC) could not be read from here.
  - The datasheet also cautions that removing VCC during an RF command can abort that command.
- **I2C lines with the tag unpowered.** SDA and SCL are rated −0.5 to 6.5 V regardless of VCC (DS10925 Table 242). An ST engineer states the tag does not load the bus at VCC = 0.
- **Tap-to-wake.**
  - The tag's open-drain GPO works with VCC off as long as the RF field is present and the pull-up is powered (DS10925 §5.2, Tables 23 and 24). The pull-up must be above 4.7 kΩ (§2.4.2); the ATtiny's internal pull-up is 20 to 50 kΩ (Table 36-16).
  - The default GPO mode is RF field change (§2.4.2). Only the rising edge, a phone arriving, is reported without VCC.
  - PC2 is a fully asynchronous pin and can wake the MCU from power-down (DS40002204A §16.3.3.1).
  - Every accidental wake from a nearby phone costs one show.

## Issue 3: nothing limits the LED current

**Board.** All 24 anodes are on VCC; each bank's cathodes go to one CJ3400 FET's drain. There are no resistors.

- **History:** the first BOM (commit `aaac39e`, 2026-05-16) had R1 to R6, 10 kΩ arrays (LCSC C25725). Commit `183b52e` (2026-05-21, "PCB File Updated") removed them and swapped the ZTS6156 mic for the LMA2718B381-OA7.

**Measured.** 25.5 mA with one 4-LED bank latched on (Run 1).

**Risk, unmeasured.** The ATtiny1616 is specified for 10 MHz only at 2.7 V and above (DS40002204A, Table 36-3). The brown-out detector is off (`firmware/platformio.ini`). If a lit bank pulls the rail below 2.7 V, the MCU runs out of spec without resetting.

**Decision so far:** no resistors, to keep the board simple. Zero-part mitigation: run the MCU at 5 MHz, which is in spec down to 1.8 V.

## Issue 4: copper under the cell's outer edge

From the V0-3 board file:

- **Negative pad:** BT1's negative contact is a 14 mm circle at the cell's centre. The cell is 20 mm across, so its outer 3 mm sits over other copper. On a CR2032 that edge is where the negative cap, the seal and the crimp of the positive can meet.
- **Ground pour:** covers most of that outer ring, 8 to 10 mm from the centre.
- **Vias:** seven ground vias sit 7.9 to 8.7 mm from the centre.
- **VCC track:** one runs 8.8 mm from the centre.
- **Insulation:** the vias are tented, so solder mask is all that separates any of this copper from the cell.
- **Exposed copper:** ground lettering with no solder mask sits 0.4 mm outside the cell's outline at the closest point.

Whether the ground copper or the VCC track is the one at risk depends on which part of the cell sits over it: ground copper under the positive crimp, or VCC copper under the negative cap. The 0.0 mA readings rule out a short above about 1 mA on this card. A smaller leak cannot be excluded.

**Fix for V0-3:** keep copper out of the cell's footprint except the negative pad and its vias.

## Issue 5: docs that no longer match the board

- `firmware/LEARNINGS.md:218`: "LED resistor is 10 kΩ"; also `firmware/LEARNINGS.md:13` and `firmware/bringup/src/config.h:24`.
- `firmware/production-pond/src/pins.h:27` and `firmware/production-nfc/src/pins.h:27`: "individual current-limit resistors to VCC".
- `firmware/production-pond/src/mic.cpp:8` and `firmware/LEARNINGS.md:105`, `:301`: describe the ZTS6156 mic.
- `firmware/LEARNINGS.md:157`: the sleep example teaches the pattern from Issue 1.

## Cell life reference

Nominal CR2032 capacity is about 220 mAh. Real capacity under 25 mA pulses is lower.

| Continuous draw | Approximate life |
|---|---|
| 3.7 mA | 2.5 days |
| 3.0 mA | 3 days |
| 0.2 mA | 6.5 weeks |
| 50 µA | 6 months |
| 10 µA | 2.5 years |

## Sources

- [ATtiny1614/1616/1617 datasheet, DS40002204A](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/ATtiny1614-16-17-DataSheet-DS40002204A.pdf)
- [ATtiny1614/1616/1617 silicon errata, DS80000886C](https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny1614-16-17-SilConErrataClarif-DS80000886C.pdf)
- [megaTinyCore errata notes](https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/Errata.md)
- [ST25DV04K/16K/64K datasheet, DS10925 Rev 9 (SparkFun copy)](https://cdn.sparkfun.com/assets/8/7/4/5/3/en.DM00167716.pdf)
- [ST25DV04K errata, ES0616](https://www.st.com/resource/en/errata_sheet/es0616-st25dv04k-st25dv16k-and-st25dv64k-device-limitations-stmicroelectronics.pdf) (not read)
- [ST25DVxxKC errata, ES0617](https://www.st.com/resource/en/errata_sheet/es0617-st25dv04kc-st25dv16kc-and-st25dv64kc-device-limitations-stmicroelectronics.pdf) (seen through search results only)
- [ST forum: VCC rise time with a GPIO-powered tag](https://community.st.com/st25-nfc-rfid-tags-and-readers-54/power-management-timing-parameters-of-the-st25dv-chip-168969)
- [ST forum: I2C bus load with VCC at 0](https://community.st.com/st25-nfc-rfid-tags-and-readers-54/st25dv64k-ier6s3-load-on-i2c-bus-with-vcc-0-41023)
- [Pokit Meter (POK-TRK) specifications, RobotShop](https://www.robotshop.com/products/pokit-meter-wireless-multimeter-dso-logger-transparent)
