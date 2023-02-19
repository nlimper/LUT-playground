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
