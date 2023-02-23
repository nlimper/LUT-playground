#include "epd.h"
#include "7segment.h"
#include "GxEPDextra.h"
#include "randomtext.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFi.h>
#include "waveforms.h"

GxEPDextra display;
EPDraw EPD;
struct waveform currentwaveform;
extern TM1637TinyDisplay6 led;

// CMD_DISP_UPDATE_CTRL2: bitmapped enabled phases of the update: (in this order)
//   80 - enable clock signal
//   40 - enable CP
//   20 - load temperature value
//   10 - load LUT
//   08 - initial display
//   04 - pattern display
//   02 - disable CP
//   01 - disable clock signal

void readLUT(struct waveform *waveform, int8_t temperature) {

	led.showString("busy");
	while (EPD.isBusy()) {
	}
	led.clear();

	display.init();
	EPD.writeCommand(CMD_DISP_UPDATE_CTRL2);
	EPD.writeData(0xf8);
	EPD.writeCommand(CMD_ACTIVATION);
	EPD.waitWhileBusy("_PowerOn", 5000);

	EPD.writeCommandData((const uint8_t[]){CMD_TEMP_SENSOR_CONTROL, 0x48}, 2);	   // external temp sensor

	EPD.writeCommand(CMD_TEMP_SENSOR_WRITE);
	EPD.writeData(temperature);
	EPD.writeData(0x00);

	EPD.writeCommandData((const uint8_t[]){CMD_DISP_UPDATE_CTRL2, 0xB1}, 2);	   // mode 1 (i2C)
	EPD.writeCommand(CMD_ACTIVATION);
	EPD.waitWhileBusy("_loadLUT", 5000);

	uint8_t *waveformbuffer = (uint8_t *)waveform;
	EPD.readData(CMD_READ_LUT, waveformbuffer, (SCREEN_LUT_LENGTH * 10) + 6);
	display.powerOff();
	currentwaveform = *waveform;
}

void printWaveform(struct waveform *waveform) {
	Serial.printf("gatelevel %02X\n", waveform->gatelevel);
	Serial.printf("dummyline %02X\n", waveform->dummyline);
	Serial.printf("gatewidth %02X\n", waveform->gatewidth);
	Serial.printf("sourcelevel0 %02X\n", waveform->sourcelevel[0]);
	Serial.printf("sourcelevel1 %02X\n", waveform->sourcelevel[1]);
	Serial.printf("sourcelevel2 %02X\n", waveform->sourcelevel[2]);
	for (int j = 0; j < SCREEN_LUT_LENGTH; j++) {
		Serial.printf("group %d\n", j);
		Serial.printf("  phaselength A %02X\n", waveform->group[j].phaselength[0]);
		Serial.printf("  phaselength B %02X\n", waveform->group[j].phaselength[1]);
		Serial.printf("  phaselength C %02X\n", waveform->group[j].phaselength[2]);
		Serial.printf("  phaselength D %02X\n", waveform->group[j].phaselength[3]);
		Serial.printf("  repeat %02X\n", waveform->group[j].repeat);
		for (int i = 0; i < 5; i++) {
			Serial.printf("  lut %d\n", i);
			Serial.printf("    level A %02X\n", waveform->lut[i].group[j].A);
			Serial.printf("    level B %02X\n", waveform->lut[i].group[j].B);
			Serial.printf("    level C %02X\n", waveform->lut[i].group[j].C);
			Serial.printf("    level D %02X\n", waveform->lut[i].group[j].D);
		}
	}
}

void useLut(struct waveform *wf) {
	currentwaveform = *wf;
}

void writeLut(struct waveform *wf) {

	if (EPD.isBusy()) {
		led.showString("busy");
		while (EPD.isBusy()) {
		}
		led.clear();
	}

	EPD.writeCommandData((const uint8_t[]){CMD_DISP_UPDATE_CTRL2, 0xB1}, 2);
	EPD.writeCommand(CMD_ACTIVATION);
	EPD.waitWhileBusy("_loadLUT", 5000);

	uint8_t *waveformbuffer = (uint8_t *)wf;
	EPD.writeCommand(CMD_WRITE_LUT);
	EPD.writeData(waveformbuffer, (SCREEN_LUT_LENGTH * 10)); // +6
	currentwaveform = *wf;
}

void initEpd() {

	display.init();
	display.setFullWindow();
	display.setRotation(1);
	display.fillScreen(GxEPD_WHITE);
	display.setFont(&FreeSans9pt7b);
	display.setTextSize(1);
	display.setTextColor(GxEPD_BLACK);
	drawCentreString("LUT playground", 148, 50);

	display.setTextColor(GxEPD_BLACK);
	display.setCursor(20, 20);
	display.print("IP Address: ");
	display.print(WiFi.localIP());

	display.fillRect(150, 70, 30, 30, GxEPD_BLACK);
	display.drawRect(190, 70, 30, 30, GxEPD_BLACK);
	//display.fillRect(230, 70, 30, 30, GxEPD_RED);

	//display.display();
	memcpy(&currentwaveform, &quickwaveform, sizeof(waveform));
	
	displayWithProgress(1);
	display.powerOff();

	struct waveform waveformBuffer;
	struct waveform *waveform = &waveformBuffer;
	readLUT(waveform, 30);
}

void displayWithProgress(uint8_t cmd) {

	if (EPD.isBusy()) {
		led.showString("busy");
		while (EPD.isBusy()) {
		}
		led.clear();
	}

	if (cmd == 0) {
		struct waveform waveformBuffer;
		struct waveform *waveform = &waveformBuffer;
		readLUT(waveform, 30);
	}

	EPD.writeCommand(CMD_SOFT_RESET);
	while (EPD.isBusy()) {
	}

	EPD.writeCommand(0x3B);
	EPD.writeData(currentwaveform.gatewidth);

	EPD.writeCommandData((const uint8_t[]){CMD_ANALOG_BLK_CTRL, 0x54}, 2);
	EPD.writeCommandData((const uint8_t[]){CMD_DIGITAL_BLK_CTRL, 0x3B}, 2);
	// EPD.writeCommandData((const uint8_t[]){CMD_SOFT_START_CTRL, 0x8f, 0x8f, 0x8f, 0x3f}, 5);
	EPD.writeCommandData((const uint8_t[]){CMD_DRV_OUTPUT_CTRL, (SCREEN_HEIGHT - 1) & 0xff, (SCREEN_HEIGHT - 1) >> 8, 0x00}, 4);

	EPD.writeCommandData((const uint8_t[]){CMD_BORDER_WAVEFORM_CTRL, 0xC0}, 2);

	writeLut(&currentwaveform);

	if (cmd == 2) {
		// incremental LUT: copy old black to new red buffer
		EPD.writeImage2(display.getBlackBuffer(), display.getBlackBufferOld(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		EPD.writeImage2(display.getBlackBuffer(), display.getColorBuffer(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	EPD.writeCommand(0x03);
	EPD.writeData(currentwaveform.gatelevel);

	EPD.writeCommand(0x04);
	EPD.writeData(currentwaveform.sourcelevel[0]);
	EPD.writeData(currentwaveform.sourcelevel[1]);
	EPD.writeData(currentwaveform.sourcelevel[2]);

	EPD.writeCommand(0x3A);
	EPD.writeData(currentwaveform.dummyline);

	EPD.writeCommand(0x3B);
	EPD.writeData(currentwaveform.gatewidth);

	EPD.writeCommand(0x22);
	EPD.writeData(0xC7);
	EPD.writeCommand(0x20);

	uint64_t waitmicros = micros();

	led.setBrightness(BRIGHT_HIGH);
	led.clear();

	int frequency = getFrequency(currentwaveform.dummyline, currentwaveform.gatewidth);

	waitmicros += 4 * 20000;
	do {
	} while (micros() < waitmicros);
	uint32_t t = millis();
	const char *phasename[] = {"A", "B", "C", "D"};
	for (uint8_t wavegroup = 0; wavegroup < 7; wavegroup++) {
		led.showNumber(wavegroup, false, 1, 0);
		uint8_t repeat = currentwaveform.group[wavegroup].repeat + 1;
		for (uint8_t repetition = 0; repetition < repeat; repetition++) {
			led.showNumber(repeat - repetition, false, 3, 3);
			for (uint8_t phase = 0; phase < 4; phase++) {
				uint8_t frames = currentwaveform.group[wavegroup].phaselength[phase];
				if (frames) {
					led.showString(phasename[phase], 1, 1, 0);
					waitmicros += frames * (1000000 / frequency);
					do {
					} while (micros() < waitmicros);
				}
			}
		}
	}
	t = millis() - t;
	led.clear();
	while (EPD.isBusy()) {	}
	led.showNumber((long)t);
	display.powerOff();
	display.backupBuffer();
}

void displaySolo(uint8_t sologroup) {
	for (int j = 0; j < 7; j++) {
		if (j != sologroup) {
			currentwaveform.group[j].phaselength[0] = 0;
			currentwaveform.group[j].phaselength[1] = 0;
			currentwaveform.group[j].phaselength[2] = 0;
			currentwaveform.group[j].phaselength[3] = 0;
			currentwaveform.group[j].repeat = 0;
		}
	}
	displayWithProgress(1);
}

void drawCentreString(const String &buf, int x, int y) {
	int16_t x1, y1;
	uint16_t w, h;
	display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); 
	display.setCursor(x - (w / 2), y);
	display.print(buf);
}

String wave2json(struct waveform *wf) {
	DynamicJsonDocument doc(8000);

	JsonArray lutArray = doc.createNestedArray("lut");
	for (int i = 0; i < 5; i++) {
		JsonArray groupArray = lutArray.createNestedArray();
		for (int j = 0; j < 7; j++) {
			JsonObject vgroupObj = groupArray.createNestedObject();
			vgroupObj["A"] = wf->lut[i].group[j].A;
			vgroupObj["B"] = wf->lut[i].group[j].B;
			vgroupObj["C"] = wf->lut[i].group[j].C;
			vgroupObj["D"] = wf->lut[i].group[j].D;
		}
	}

	JsonArray groupArray = doc.createNestedArray("group");
	for (int i = 0; i < 7; i++) {
		JsonObject groupObj = groupArray.createNestedObject();
		JsonArray phaselengthArray = groupObj.createNestedArray("phaselength");
		for (int j = 0; j < 4; j++) {
			phaselengthArray.add(wf->group[i].phaselength[j]);
		}
		groupObj["repeat"] = wf->group[i].repeat;
	}

	doc["vgh"] = wf->gatelevel;
	doc["vsh1"] = wf->sourcelevel[0];
	doc["vsh2"] = wf->sourcelevel[1];
	doc["vsl"] = wf->sourcelevel[2];
	doc["dummyline"] = wf->dummyline;
	doc["gatewidth"] = wf->gatewidth;

	String jsonString;
	serializeJson(doc, jsonString);
	return jsonString;
}

waveform json2wave(const JsonObject &doc) {
	waveform my_waveform;
	for (size_t i = 0; i < 5; i++) {
		for (size_t j = 0; j < 7; j++) {
			my_waveform.lut[i].group[j].A = doc["lut"][i][j]["A"];
			my_waveform.lut[i].group[j].B = doc["lut"][i][j]["B"];
			my_waveform.lut[i].group[j].C = doc["lut"][i][j]["C"];
			my_waveform.lut[i].group[j].D = doc["lut"][i][j]["D"];
		}
	}

	for (size_t i = 0; i < 7; i++) {
		for (size_t j = 0; j < 4; j++) {
			my_waveform.group[i].phaselength[j] = doc["group"][i]["phaselength"][j];
		}
		my_waveform.group[i].repeat = doc["group"][i]["repeat"];
	}

	my_waveform.gatelevel = doc["vgh"];
	my_waveform.sourcelevel[0] = doc["vsh1"];
	my_waveform.sourcelevel[1] = doc["vsh2"];
	my_waveform.sourcelevel[2] = doc["vsl"];
	my_waveform.dummyline = doc["dummyline"];
	my_waveform.gatewidth = doc["gatewidth"];
	return my_waveform;
}

uint8_t sendPattern(uint8_t patternid) {
	if (patternid == 0) return 0;

	if (EPD.isBusy()) {
		led.showString("busy");
		while (EPD.isBusy()) {
		}
		led.clear();
	}

	display.init();
	display.setFullWindow();
	display.setRotation(1);
	display.fillScreen(GxEPD_WHITE);
	uint16_t color[] = {GxEPD_WHITE, GxEPD_BLACK, GxEPD_RED, GxEPD_DARKGREY};

	switch (patternid) {
	case TEST_WHITE:
		break;

	case TEST_PATTERN1:
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				display.fillRect(80+x*30, y*30, 27, 27, color[x]);
			}
		}
		break;

	case TEST_PATTERN2:
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				display.fillRect(80 + x * 30, y * 30, 27, 27, color[y]);
			}
		}
		break;

	case TEST_LOREMIPSUM:
		display.setCursor(0, 2);
		display.setTextColor(GxEPD_BLACK, GxEPD_WHITE);
		display.setTextSize(1);
		display.setFont();
		display.setTextWrap(true);
		display.print("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas quis velit libero. Praesent id turpis sapien. Integer aliquet varius tortor vitae feugiat. Donec sit amet varius velit.");
		display.setTextColor(GxEPD_RED, GxEPD_WHITE);
		display.print("Donec urna dui, suscipit ut elit a, hendrerit pellentesque urna. Nunc maximus, nunc et facilisis suscipit, mauris nisl volutpat mauris, vitae placerat odio metus vel dui.");
		display.setTextColor(GxEPD_DARKGREY, GxEPD_WHITE);
		display.print("Donec urna dui, suscipit ut elit a, hendrerit pellentesque urna. Nunc maximus, nunc et facilisis suscipit, mauris nisl volutpat mauris, vitae placerat odio metus vel dui.");
		display.setTextColor(GxEPD_WHITE, GxEPD_BLACK);
		display.print("Donec urna dui, suscipit ut elit a, hendrerit pellentesque urna.");
		break;

	case TEST_RANDOMTEXT:
		display.setCursor(0, 2);
		display.setTextColor(GxEPD_BLACK, GxEPD_WHITE);
		display.setTextSize(1);
		display.setFont();
		display.setTextWrap(true);
		display.print(generate_random_string());
		display.setTextColor(GxEPD_RED, GxEPD_WHITE);
		display.print(generate_random_string());
		break;
	}

	return 0;
}

int getFrequency(int dummyline, int gatewidth) {
	int closestFrequency = 0;
	int closestDifference = 255;

	int frequencyData[][3] = {{15, 121, 14}, {20, 16, 14}, {25, 38, 13}, {30, 78, 12}, {35, 24, 12}, {40, 67, 11}, {45, 26, 11}, {50, 44, 10}, {55, 13, 10}, {60, 33, 9}, {65, 7, 9}, {70, 40, 8}, {75, 17, 8}, {80, 47, 7}, {85, 26, 7}, {90, 8, 7}, {95, 50, 6}, {100, 33, 6}, {105, 17, 6}, {110, 3, 6}, {115, 34, 5}, {120, 20, 5}, {125, 7, 5}, {135, 36, 4}, {140, 24, 4}, {145, 13, 4}, {150, 3, 4}, {155, 39, 3}, {160, 28, 3}, {165, 18, 3}, {170, 9, 3}, {175, 0, 3}, {180, 47, 2}, {185, 37, 2}, {190, 28, 2}, {195, 20, 2}, {200, 12, 2}};

	int length = sizeof(frequencyData) / sizeof(frequencyData[0]);

	for (int i = 0; i < length; i++) {
		if (frequencyData[i][2] != gatewidth) {
			continue;
		}

		int difference = abs(frequencyData[i][1] - dummyline);
		if (difference < closestDifference) {
			closestFrequency = frequencyData[i][0];
			closestDifference = difference;
		}
	}

	return closestFrequency;
}
