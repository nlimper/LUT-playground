#include <Arduino.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#include <GxEPD2_BW.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 296
#define SCREEN_LUT_LENGTH 7

#define LUTGROUP_NEGATIVE 0
#define LUTGROUP_FASTBLINK 1
#define LUTGROUP_SLOWBLINK 2
#define LUTGROUP_SET 3
#define LUTGROUP_IMPROVE_SHARPNESS 4
#define LUTGROUP_IMPROVE_REDS 5
#define LUTGROUP_UNUSED 6

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
	EPDraw() : GxEPD2_290_C90c(/*CS=5*/ 5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4) {}
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
};

class GxEPDextra : public GxEPD2_3C<EPDraw, GxEPD2_290_C90c::HEIGHT> {
public:
	GxEPDextra() : GxEPD2_3C<EPDraw, GxEPD2_290_C90c::HEIGHT>(EPDraw()) {};
	uint8_t *getBlackBuffer() { return _black_buffer; };
	uint8_t *getColorBuffer() { return _color_buffer; };
};
