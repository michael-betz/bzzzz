#include<Arduino.h>

#define TOP 0xFF
// dead-zone near TOP to avoid glitches
// 16 bit access: 0x18
// 8 bit access: 0xF
#define MAX_PWR (TOP - 0xF)

// #define PIN_PWM_A 6  // OC0A
// #define PIN_PWM_B 5  // OC0B
#define PIN_PWM_A 9  // OC1A
#define PIN_PWM_B 10  // OC1B
#define PIN_AD 8  // Arc detector

void set_phase(uint8_t v)
{
	if (v > MAX_PWR)
		v = MAX_PWR;
	cli();
	while(TCNT1L <= MAX_PWR) {}
	OCR1BL = v;
	sei();
}

void setup()
{
	pinMode(PIN_PWM_A, OUTPUT);
	pinMode(PIN_PWM_B, OUTPUT);
	pinMode(PIN_AD, INPUT);

	// Timer 0: toggle at fixed frequency and phase
	OCR1A = 0;
	OCR1B = 0;
	ICR1 = TOP;
	TCCR1A = _BV(COM1B0) | _BV(COM1A0);
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS00);  // only CS00 = no prescaler

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

	Serial.println(OCR1B);
}
