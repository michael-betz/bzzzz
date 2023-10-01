#include<Arduino.h>

#define MAX_PWR 0xF0  // need dead-zone near TOP to avoid glitches

#define PIN_PWM_A 6  // OC0A
#define PIN_PWM_B 5  // OC0B
// #define PIN_PWM_B 9  // OC1A
// #define PIN_PWM_B 10  // OC1B
#define PIN_AD 8  // Arc detector

void set_phase(uint8_t v)
{
	if (v > MAX_PWR)
		v = MAX_PWR;
	cli();
	while(TCNT0 <= MAX_PWR) {}
	OCR0B = v;
	sei();
}

void setup()
{
	pinMode(PIN_PWM_A, OUTPUT);
	pinMode(PIN_PWM_B, OUTPUT);
	pinMode(PIN_AD, INPUT);

	// Timer 0: toggle at fixed frequency and phase
	OCR0A = 0;
	OCR0B = 0;
	TCCR0A = _BV(COM0B0) | _BV(COM0A0);  // TOP = 0xFF
	TCCR0B = _BV(CS00);  // only CS00 = no prescaler

	Serial.begin(115200);
	Serial.print("Yo! This is Bzzzz2!\n");

	set_phase(0x30);
}

void loop()
{
	static uint8_t val = 0;
	int c = Serial.read();
	if (c == -1) {
		return;
	}
	if (c == '=' && val < MAX_PWR) {
		val++;
	} else if (c == '-' && val > 0) {
		val--;
	}
	set_phase(val);

	Serial.println(OCR0B);
}
