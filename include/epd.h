#include <Arduino.h>
#include "ArduinoJson.h"

#define CMD_DRV_OUTPUT_CTRL 0x01
#define CMD_SOFT_START_CTRL 0x0C
#define CMD_ENTER_SLEEP 0x10
#define CMD_DATA_ENTRY_MODE 0x11
#define CMD_SOFT_RESET 0x12
#define CMD_SOFT_RESET2 0x13
#define CMD_SETUP_VOLT_DETECT 0x15
#define CMD_TEMP_SENSOR_CONTROL 0x18
#define CMD_TEMP_SENSOR_WRITE 0x1A
#define CMD_ACTIVATION 0x20
#define CMD_DISP_UPDATE_CTRL 0x21
#define CMD_DISP_UPDATE_CTRL2 0x22
#define CMD_WRITE_FB_BW 0x24
#define CMD_WRITE_FB_RED 0x26
#define CMD_UNKNOWN_1 0x2B
#define CMD_LOAD_OTP_LUT 0x31
#define CMD_WRITE_LUT 0x32
#define CMD_READ_LUT 0x33
#define CMD_BORDER_WAVEFORM_CTRL 0x3C
#define CMD_WINDOW_X_SIZE 0x44
#define CMD_WINDOW_Y_SIZE 0x45
#define CMD_WRITE_PATTERN_RED 0x46
#define CMD_WRITE_PATTERN_BW 0x47
#define CMD_XSTART_POS 0x4E
#define CMD_YSTART_POS 0x4F
#define CMD_ANALOG_BLK_CTRL 0x74
#define CMD_DIGITAL_BLK_CTRL 0x7E

#define SCREEN_CMD_CLOCK_ON 0x80
#define SCREEN_CMD_CLOCK_OFF 0x01
#define SCREEN_CMD_ANALOG_ON 0x40
#define SCREEN_CMD_ANALOG_OFF 0x02
#define SCREEN_CMD_LATCH_TEMPERATURE_VAL 0x20
#define SCREEN_CMD_LOAD_LUT 0x10
#define SCREEN_CMD_USE_MODE_2 0x08 // modified commands 0x10 and 0x04
#define SCREEN_CMD_REFRESH 0xC7

#define TEST_NA 0
#define TEST_WHITE 1
#define TEST_PATTERN1 2
#define TEST_PATTERN2 3
#define TEST_LOREMIPSUM 4
#define TEST_RANDOMTEXT 5

void initEpd();
void readLUT(struct waveform *waveform, int8_t temperature);
void writeLut(struct waveform *wf);
void useLut(struct waveform *wf);
void drawCentreString(const String &buf, int x, int y);
String wave2json(struct waveform *wf);
waveform json2wave(const JsonObject &doc);
void displayWithProgress(uint8_t cmd);
void displaySolo(uint8_t sologroup);
uint8_t sendPattern(uint8_t patternid);
int getFrequency(int dummyline, int gatewidth);
