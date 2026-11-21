#include "sleepy.h"

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "leds.h"
#include "pins.h"

// 16-bit so the pond variant's 300 s live-URL window fits. The ISR writes
// it and the main context reads it, so every main-context access is inside
// a cli()/sei() pair -- on an 8-bit core a 16-bit load is two instructions
// and can tear across the decrement, which would read a false zero and end
// the sleep early.
static volatile uint16_t s_pit_ticks_remaining = 0;

ISR(RTC_PIT_vect) {
    RTC.PITINTFLAGS = RTC_PI_bm;
    if (s_pit_ticks_remaining > 0) s_pit_ticks_remaining--;
}

static void disable_all_input_buffers() {
    // LED bank pins -- still drivable as outputs with input buffer off.
    for (uint8_t i = 0; i < NUM_BANKS; i++) {
        volatile uint8_t* pinctrl = getPINnCTRLregister(
            digitalPinToPortStruct(LED_BANK_PINS[i]),
            digitalPinToBitPosition(LED_BANK_PINS[i]));
        if (pinctrl) *pinctrl = PORT_ISC_INPUT_DISABLE_gc;
    }
    // I2C lines: ndef_deinit() leaves PULLUPEN set. Hard-disable here so
    // terminal sleep pays for neither the pullup current nor a floating
    // Schmitt input.
    PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTB.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTC.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    // PA0 is UPDI -- leave alone.
}

/*
 * ══ START THE PIT ONCE, AND NEVER WAIT ON RTC.STATUS ══
 * This used to open with `while (RTC.STATUS != 0) {}`, start the PIT, and
 * stop it again on the way out.
 *
 * What was measured (Nov 2026): after a great-luck reveal the card drew a
 * dead-steady 3.7 mA with the LEDs off, where this function should have
 * had it at a fraction of that. 3.7 mA is the CPU flat out at 10 MHz.
 * Great luck and little luck call this for their 12 s hold, so their
 * 300 s live-URL wait is the SECOND call in a boot.
 *
 * The explanation that fits: on the tinyAVR 0/1-series the PIT and the
 * RTC are not independent (erratum "Disabling the RTC Stops the PIT"),
 * and megaTinyCore's errata notes describe exactly this sequence -- PIT
 * run with the RTC itself disabled, then a `while (RTC.STATUS)` -- ending
 * with RTC_CNTBUSY stuck at 1 and the wait never returning. With the LEDs
 * off, a spinning CPU looks the same as a sleeping one.
 *
 * So: no RTC.STATUS wait (those busy bits belong to RTC registers this
 * code never writes), and the PIT is started once and left counting, which
 * is the vendor's workaround. Each call only unmasks its interrupt.
 */
static void pit_start_once() {
    if (RTC.PITCTRLA & RTC_PITEN_bm) return;
    RTC.CLKSEL   = RTC_CLKSEL_INT32K_gc;
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;  // 1 Hz
}

void sleep_timed_seconds(sleep_seconds_t seconds) {
    if (seconds == 0) return;

    pit_start_once();

    // The PIT free-runs between calls, so a call after the first starts
    // somewhere inside a second and can come back up to 1 s early. Nothing
    // here is timed tighter than that.
    cli();
    s_pit_ticks_remaining = seconds;
    RTC.PITINTFLAGS = RTC_PI_bm;  // a tick from while we were masked is stale
    RTC.PITINTCTRL  = RTC_PI_bm;
    sei();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // Classic AVR race-free sleep: test the flag with interrupts off, then
    // sei() immediately before sleep_cpu(). The instruction after sei() is
    // guaranteed to execute before any pending interrupt, so the PIT can't
    // fire in the window between the test and the sleep and leave us
    // asleep for a whole extra second (or, at the last tick, forever).
    for (;;) {
        cli();
        if (s_pit_ticks_remaining == 0) {
            sei();
            break;
        }
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
    }

    // Mask the interrupt so it can't wake a later sleep. The PIT itself
    // keeps counting -- see the note above pit_start_once().
    RTC.PITINTCTRL = 0;
}

[[noreturn]] void sleep_forever() {
    bank_all_off();
    // megaTinyCore's Arduino init() leaves ADC0 enabled for analogRead();
    // its bias current would dominate the < 1 uA sleep target.
    ADC0.CTRLA &= ~ADC_ENABLE_bm;
    ADC1.CTRLA &= ~ADC_ENABLE_bm;
    disable_all_input_buffers();

    // Nothing needs the PIT again, so this is the one place it is safe to
    // stop it: it takes the 32 kHz oscillator down with it (~1 uA).
    RTC.PITINTCTRL = 0;
    RTC.PITCTRLA   = 0;

    cli();  // no wake source desired -- stay asleep until coin pull
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    for (;;) sleep_cpu();
}
