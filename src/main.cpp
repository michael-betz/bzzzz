#include<Arduino.h>
#include "perlin.h"

#define PIN_PWM 3
int seed1;

unsigned last_val = 0;

// max power for val = 0xFF
void set_power(int val)
{
	if (val > 0xFF)
		val = 0xFF;
	if (val < 5)
		val = 5;

	int err = (val << 7) - last_val;
	last_val += (err >> 5) + (err > 0 ? 1 : err < 0 ? -1 : 0);  // >> 5 sets filter speed

	val = (last_val >> 8);

	// if (val <= 0)
	// 	TCCR2A &= ~(1 << COM2B1);  // disable PWM
	// else
	// 	TCCR2A |= 1 << COM2B1;  // enable PWM

	OCR2B = val;
}

void setup()
{
	pinMode(PIN_PWM, OUTPUT);
	pinMode(6, INPUT_PULLUP);

	TCCR2A = _BV(WGM21) | _BV(WGM20) | _BV(COM2B1);
	TCCR2B = _BV(WGM22) | _BV(CS20);
	OCR2A = 0xFF;
	OCR2B = 1;

	randomSeed(analogRead(0));
	seed1 = random();

	Serial.begin(115200);
	Serial.print("Yo! This is Bzzzz!\nseed: ");
	Serial.println(seed1);
}

void loop()
{
	// static int pwr = 0;
	static int tick = 0;
	// static bool is_auto = true;

	delay(50);

	double n1 = pnoise1d(0.0002 * tick++, 0.7, 5, seed1);
	n1 = n1 * n1 * 120;

	// if (is_auto)
	// 	pwr = n1;

	// int c = Serial.read();
	// if (c == -1) {
	// 	set_power(pwr);
	// 	return;
	// } else if (c == '=' && pwr < 0xFF) {
	// 	pwr++;
	// } else if (c == '-' && pwr > 0) {
	// 	pwr--;
	// } else if (c == '0') {
	// 	pwr = 0;
	// } else if (c == '2' && OCR2A > 0) {
	// 	OCR2A--;
	// } else if (c == '1' && OCR2A < 0xFF) {
	// 	OCR2A++;
	// } else if (c == 'q') {
	// 	is_auto = !is_auto;
	// }

	// Serial.print("pwr: ");
	// Serial.print(pwr);

	// Serial.print("   f: ");
	// Serial.print(OCR2A);

	// Serial.print("\n");

	set_power(n1);
	// set_power(0xFF);

	Serial.print(n1);
	Serial.print("   ");
	Serial.println(OCR2B);

	if (digitalRead(6) == 0) {
		OCR2B = 0;
		last_val = 0;
		Serial.print("ARC!\n");
		delay(5000);
	}
}
