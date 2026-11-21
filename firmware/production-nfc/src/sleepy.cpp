#include "sleepy.h"

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "leds.h"
#include "pins.h"

static volatile uint8_t s_pit_ticks_remaining = 0;

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
    // I2C lines: by the time sleep_forever() runs we may have left
    // them with PULLUPEN set (ndef_deinit() leaves them that way).
    // Hard-disable here so terminal sleep doesn't pay for either the
    // pullup current or a floating Schmitt input.
    PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTB.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTC.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    // PA0 is UPDI -- leave alone.
}

// Start the PIT once and never wait on RTC.STATUS.
//
// This used to open with `while (RTC.STATUS != 0) {}`, start the PIT, and
// stop it again on the way out. Measured Nov 2026: after a great-luck
// reveal the card drew a dead-steady 3.7 mA with the LEDs off -- the CPU
// flat out at 10 MHz -- where this function should have had it asleep.
// Great and little luck call this for their hold, so the live-URL wait is
// the SECOND call in a boot.
//
// The explanation that fits: on the tinyAVR 0/1-series the PIT and the
// RTC are not independent (erratum "Disabling the RTC Stops the PIT"), and
// megaTinyCore's errata notes describe this exact sequence ending with
// RTC_CNTBUSY stuck at 1 and the wait never returning.
//
// So: no RTC.STATUS wait, and the PIT is started once and left counting,
// which is the vendor's workaround. Each call only unmasks its interrupt.
// See production-pond/src/sleepy.cpp for the longer note.
static void pit_start_once() {
    if (RTC.PITCTRLA & RTC_PITEN_bm) return;
    RTC.CLKSEL   = RTC_CLKSEL_INT32K_gc;
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;  // 1 Hz
}

void sleep_timed_seconds(uint8_t seconds) {
    if (seconds == 0) return;

    pit_start_once();

    // The PIT free-runs between calls, so a call after the first can come
    // back up to 1 s early. Nothing here is timed tighter than that.
    cli();
    s_pit_ticks_remaining = seconds;
    RTC.PITINTFLAGS = RTC_PI_bm;  // a tick from while we were masked is stale
    RTC.PITINTCTRL  = RTC_PI_bm;

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    while (s_pit_ticks_remaining > 0) sleep_cpu();
    sleep_disable();

    // Mask the interrupt so it can't wake a later sleep. The PIT itself
    // keeps counting.
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
