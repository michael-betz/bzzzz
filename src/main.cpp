#include "simplex.h"
#include <Arduino.h>
#include <stdint.h>

#define TOP 0xFF

#define N_DITHER_BITS 4

// Original PCB (maybe?)
// #define PIN_PWM_A 6  // OC0A
// #define PIN_PWM_B 5  // OC0B

// PCB with botch wires
#define PIN_PWM_A 9   // OC1A
#define PIN_PWM_B 10  // OC1B
#define PIN_AD 8      // Arc detector

static volatile uint8_t target_base;
static volatile uint8_t fraction;

void set_phase(uint16_t val) {
    if (val > (TOP << N_DITHER_BITS))
        val = TOP << N_DITHER_BITS;
    cli();
    target_base = val >> N_DITHER_BITS;
    fraction = val & ((1 << N_DITHER_BITS) - 1);
    sei();
}

// This interrupt fires exactly when the OCR1B pin toggles
// We have 1 counter cycle to calculate the next value for OCR1B
ISR(TIMER1_COMPB_vect) {
    static uint8_t current_base_phase = 0;
    static uint8_t accumulator = 0;

    // --- The Anti-Inversion Sync Logic ---
    if (target_base > current_base_phase) {
        // SLEW UP safely: Limit increases to +1 per timer cycle.
        // Because entering this ISR takes ~15 clock cycles, the timer is
        // already ~15 ticks past the old value. By only adding 1, the new
        // OCR1B is physically "behind" the timer, guaranteeing it will wait
        // for the next cycle instead of double-toggling right now.
        current_base_phase++;
    } else {
        // SLEW DOWN safely: Decreasing the value instantly is mathematically
        // immune to double-toggling. We can jump instantly.
        current_base_phase = target_base;
    }

    // Accumulate the fraction
    accumulator += fraction;

    // Add its overflow to the hardware-pwm value
    uint8_t dithered_phase = current_base_phase + (accumulator >> N_DITHER_BITS);
    // Reset the overflow in the accumulator
    accumulator &= ((1 << N_DITHER_BITS) - 1);

    // Write to the hardware
    OCR1B = dithered_phase;
}

void setup() {
    pinMode(PIN_PWM_A, OUTPUT);
    pinMode(PIN_PWM_B, OUTPUT);
    pinMode(PIN_AD, INPUT);

    // Timer 0: toggle at fixed frequency and phase
    OCR1A = 0;
    OCR1B = 0;
    ICR1 = TOP;
    TCCR1A = _BV(COM1B0) | _BV(COM1A0);  // toggle pins on compare match
    TCCR1B =
        _BV(WGM13) | _BV(WGM12) | _BV(CS00);  // no prescaler, ICR1 determines TOP value (frequency)
    // Enable the Timer1 Compare Match B Interrupt
    TIMSK1 |= _BV(OCIE1B);

    set_phase(0);

    Serial.begin(115200);
    Serial.print("Yo! This is Bzzzz2!\n");
}

static int16_t get_simplex_val() {
    static uint32_t frame = 0;

    // 2 octaves
    int16_t val = snoise_1D(frame << 1) >> 5;
    val += snoise_1D(frame << 3) >> 6;

    // Offset a bit
    val -= 60 << N_DITHER_BITS;

    if (val < 0)
        val = 0;

    if (val > (90 << N_DITHER_BITS))
        val = (90 << N_DITHER_BITS);

    frame++;
    return val;
}

enum { WAITING, IGNITED, STAY_HERE };

void loop() {
    static int16_t val = 0;
    static uint32_t frame = 0;
    static uint8_t state = 0;
    static uint16_t countdown = 0;

    switch (state) {
    case WAITING:
        val = get_simplex_val();
        // if (val > (76 << N_DITHER_BITS))
        //     state = IGNITED;
        break;

    case IGNITED:
        val = get_simplex_val();
        if (val <= (47 << N_DITHER_BITS)) {
            state = STAY_HERE;
            countdown = 500;
        }
        break;

    case STAY_HERE:
        if (countdown > 0)
            countdown--;
        else
            state = WAITING;
        break;

    default:
        state = WAITING;
    }

    set_phase(val);

    Serial.print(state);
    Serial.print("   ");
    Serial.print(target_base);
    Serial.print("   ");
    Serial.print(fraction);
    Serial.print("   ");
    Serial.println(OCR1B);
    frame++;

    delay(50);
}
