#include <Arduino.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#include <GxEPD2_BW.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 296
#define SCREEN_LUT_LENGTH 7

struct vgroup {
	uint8_t A : 2;
	uint8_t B : 2;
	uint8_t C : 2;
	uint8_t D : 2;
} __packed;

struct lut {
	struct vgroup group[7];
} __packed;

struct group {
	uint8_t phaselength[4];
	uint8_t repeat;
} __packed;

struct waveform {
	struct lut lut[5];
	struct group group[7];
	uint8_t gatelevel;
	uint8_t sourcelevel[3];
	uint8_t dummyline;
	uint8_t gatewidth;
} __packed;

class EPDraw : public GxEPD2_290_C90c {
public:
	EPDraw() : GxEPD2_290_C90c(/*CS=*/ 5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4) {}
	void reset();
	void waitWhileBusy(const char *comment = 0, uint16_t busy_time = 5000);
	void writeCommand(uint8_t c);
	void writeData(uint8_t d);
	void writeData(const uint8_t *data, uint16_t n);
	void writeDataPGM(const uint8_t *data, uint16_t n, int16_t fill_with_zeroes = 0);
	void writeDataPGM_sCS(const uint8_t *data, uint16_t n, int16_t fill_with_zeroes = 0);
	void writeCommandData(const uint8_t *pCommandData, uint8_t datalen);
	void writeCommandDataPGM(const uint8_t *pCommandData, uint8_t datalen);
	void startTransfer();
	void transfer(uint8_t value);
	void endTransfer();
	bool isBusy();
	void readData(uint8_t cmd, uint8_t *data, uint16_t n);
	void writeImage2(const uint8_t *black, const uint8_t *color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
};

class GxEPDextra : public GxEPD2_3C<EPDraw, GxEPD2_290_C90c::HEIGHT> {
public:
	GxEPDextra() : GxEPD2_3C<EPDraw, GxEPD2_290_C90c::HEIGHT>(EPDraw()) {};
	uint8_t *getBlackBuffer() { return _black_buffer; };
	uint8_t *getColorBuffer() { return _color_buffer; };

	uint8_t black_buffer2[(EPDraw::WIDTH / 8) * GxEPD2_290_C90c::HEIGHT];
	uint8_t color_buffer2[(EPDraw::WIDTH / 8) * GxEPD2_290_C90c::HEIGHT];

	void backupBuffer() {
		memcpy(black_buffer2, _black_buffer, sizeof(_black_buffer));
		memcpy(color_buffer2, _color_buffer, sizeof(_color_buffer));
	}

	uint8_t *getBlackBufferOld() { return black_buffer2; };
	uint8_t *getColorBufferOld() { return color_buffer2; };

	void drawPixel(int16_t x, int16_t y, uint16_t color) {
		if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;
		if (_mirror) x = width() - x - 1;
		// check rotation, move pixel around if necessary
		switch (getRotation()) {
		case 1:
			_swap_(x, y);
			x = WIDTH - x - 1;
			break;
		case 2:
			x = WIDTH - x - 1;
			y = HEIGHT - y - 1;
			break;
		case 3:
			_swap_(x, y);
			y = HEIGHT - y - 1;
			break;
		}
		// transpose partial window to 0,0
		x -= _pw_x;
		y -= _pw_y;
		// clip to (partial) window
		if ((x < 0) || (x >= int16_t(_pw_w)) || (y < 0) || (y >= int16_t(_pw_h))) return;
		// adjust for current page
		y -= _current_page * _page_height;
		// check if in current page
		if ((y < 0) || (y >= int16_t(_page_height))) return;
		uint16_t i = x / 8 + y * (_pw_w / 8);
		_black_buffer[i] = (_black_buffer[i] | (1 << (7 - x % 8))); // white
		_color_buffer[i] = (_color_buffer[i] | (1 << (7 - x % 8)));
		if (color == GxEPD_WHITE)
			return;
		if ((color == GxEPD_BLACK || color == GxEPD_DARKGREY))
			_black_buffer[i] = (_black_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
		if ((color == GxEPD_RED) || (color == GxEPD_DARKGREY))
			_color_buffer[i] = (_color_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));
	}

	template <typename T> static inline void 
	_swap_(T &a, T &b) {
		T t = a;
		a = b;
		b = t;
	};
};
