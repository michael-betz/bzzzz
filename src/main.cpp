#include<Arduino.h>

#define PIN_PWM_A 6  // OC0A
#define PIN_PWM_B 9  // OC1A
#define PIN_AD 8  // Arc detector

void setup()
{
	pinMode(PIN_PWM_A, OUTPUT);
	pinMode(PIN_PWM_B, OUTPUT);
	pinMode(PIN_AD, INPUT);

	TCCR0A = _BV(COM0A1) | _BV(WGM00);  // Phase correct, fixed frequency
	TCCR0B = _BV(CS00);  // only CS00 = no prescaler

	TCCR1A = _BV(COM1A1) | _BV(WGM10);  // Phase correct, fixed frequency
	TCCR1B = _BV(CS10);  // only CS10 = no prescaler

	Serial.begin(115200);
	Serial.print("Yo! This is Bzzzz2!\n");
}

void loop()
{
	Serial.println(OCR0A++);
	OCR1A++;
	delay(10000);
}
