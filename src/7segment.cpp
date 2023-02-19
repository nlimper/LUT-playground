#include <Arduino.h>
#include <TM1637TinyDisplay6.h>

#define CLK 13
#define DIO 15
// Initialize TM1637TinyDisplay
TM1637TinyDisplay6 led(CLK, DIO);

void initLED() {
	led.setBrightness(BRIGHT_HIGH);
	led.clear();
	led.showString("test");
}