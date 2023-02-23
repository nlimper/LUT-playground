#include <Arduino.h>
#include <TM1637TinyDisplay6.h>
#include "7segment.h"

TM1637TinyDisplay6 led(CLK, DIO);

void initLED() {
	led.setBrightness(BRIGHT_HIGH);
	led.clear();
	led.showString("ready");
}
