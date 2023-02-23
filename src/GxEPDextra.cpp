#include "GxEPDextra.h"
#include <Arduino.h>

void EPDraw::reset() {
	_reset();
}

void EPDraw::waitWhileBusy(const char *comment, uint16_t busy_time){
	_waitWhileBusy(comment, busy_time);
};

void EPDraw::writeCommand(uint8_t c) {
	_writeCommand(c);
}

void EPDraw::writeData(uint8_t d) {
	_writeData(d);
};

void EPDraw::writeData(const uint8_t *data, uint16_t n) {
	_writeData(data, n);
};

void EPDraw::writeDataPGM(const uint8_t *data, uint16_t n, int16_t fill_with_zeroes) {
	_writeDataPGM(data, n, fill_with_zeroes);
};

void EPDraw::writeDataPGM_sCS(const uint8_t *data, uint16_t n, int16_t fill_with_zeroes) {
	_writeDataPGM_sCS(data, n, fill_with_zeroes);
};

void EPDraw::writeCommandData(const uint8_t *pCommandData, uint8_t datalen) {
	_writeCommandData(pCommandData, datalen);
};

void EPDraw::writeCommandDataPGM(const uint8_t *pCommandData, uint8_t datalen) {
	_writeCommandDataPGM(pCommandData, datalen);
};

void EPDraw::startTransfer() {
	_startTransfer();
};

void EPDraw::transfer(uint8_t value){
	_transfer(value);
};

void EPDraw::endTransfer() {
	_endTransfer();
};

bool EPDraw::isBusy() {
	if (_busy >= 0) {
		delay(1); // add some margin to become active
		return (digitalRead(_busy) == _busy_level);
	} else
		Serial.println("Error: busy pin should be defined");
	return false;
}

void EPDraw::readData(uint8_t cmd, uint8_t *data, uint16_t n) {
	uint8_t _mosi = MOSI;
	uint8_t _sck = SCK;
	SPI.beginTransaction(_spi_settings);
	digitalWrite(_cs, LOW);
	digitalWrite(_dc, LOW);
	SPI.transfer(cmd);
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, HIGH);
	SPI.endTransaction();
	waitWhileBusy("_readController", 300);
	SPI.end();
	pinMode(_mosi, INPUT);
	delay(100);
	digitalWrite(_sck, HIGH);
	pinMode(_sck, OUTPUT);
	digitalWrite(_cs, LOW);
	pinMode(_mosi, INPUT);
	for (uint16_t j = 0; j < n; j++) {
		uint8_t value = 0;
		for (uint16_t i = 0; i < 8; i++) {
			digitalWrite(_sck, LOW);
			value <<= 1;
			delayMicroseconds(2);
			if (digitalRead(_mosi)) value |= 0x01;
			delayMicroseconds(2);
			digitalWrite(_sck, HIGH);
			delayMicroseconds(2);
		}
		data[j] = value;
	}
	digitalWrite(_cs, HIGH);
	pinMode(_sck, INPUT);
	SPI.begin();
}

void EPDraw::writeImage2(const uint8_t *black, const uint8_t *color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm) {
	int16_t wb = (w + 7) / 8;										// width bytes, bitmaps are padded
	x -= x % 8;														// byte boundary
	w = wb * 8;														// byte boundary
	int16_t x1 = x < 0 ? 0 : x;										// limit
	int16_t y1 = y < 0 ? 0 : y;										// limit
	int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x;	// limit
	int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
	int16_t dx = x1 - x;
	int16_t dy = y1 - y;
	w1 -= dx;
	h1 -= dy;
	if ((w1 <= 0) || (h1 <= 0)) return;

	//_setPartialRamArea(x1, y1, w1, h1);
	_writeCommand(0x44);
	_writeData(x1 / 8);
	_writeData((x1 + w1 - 1) / 8);
	_writeCommand(0x45);
	_writeData(y1 % 256);
	_writeData(y1 / 256);
	_writeData((y1 + h1 - 1) % 256);
	_writeData((y1 + h1 - 1) / 256);
	_writeCommand(0x4e);
	_writeData(x1 / 8);
	_writeCommand(0x4f);
	_writeData(y1 % 256);
	_writeData(y1 / 256);

	_writeCommand(0x24);
	for (int16_t i = 0; i < h1; i++) {
		for (int16_t j = 0; j < w1 / 8; j++) {
			uint8_t data = 0xFF;
			if (black) {
				// use wb, h of bitmap for index!
				int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
				if (pgm) {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
					data = pgm_read_byte(&black[idx]);
#else
					data = black[idx];
#endif
				} else {
					data = black[idx];
				}
				if (invert) data = ~data;
			}
			_writeData(data);
		}
	}
	_writeCommand(0x26);
	for (int16_t i = 0; i < h1; i++) {
		for (int16_t j = 0; j < w1 / 8; j++) {
			uint8_t data = 0xFF;
			if (color) {
				// use wb, h of bitmap for index!
				int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
				if (pgm) {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
					data = pgm_read_byte(&color[idx]);
#else
					data = color[idx];
#endif
				} else {
					data = color[idx];
				}
				if (invert) data = ~data;
			}
			_writeData(~data);
		}
	}
	delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}
