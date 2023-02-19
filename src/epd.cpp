#include "epd.h"
#include "7segment.h"
#include "GxEPDextra.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFi.h>
#include "randomtext.h"

// GxEPDextra<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(/*CS=5*/ 5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));
GxEPDextra display;
EPDraw EPD;
struct waveform currentwaveform;

void readLUT(struct waveform *waveform) {

	display.init();
	//Serial.println("powering on");
	EPD.writeCommand(CMD_DISP_UPDATE_CTRL2);
	EPD.writeData(0xf8);
	EPD.writeCommand(CMD_ACTIVATION);
	EPD.waitWhileBusy("_PowerOn", 5000);
	//Serial.println("ready. loading lut:");

	EPD.writeCommandData((const uint8_t[]){CMD_TEMP_SENSOR_CONTROL, 0x48}, 2);	   // external temp sensor
	EPD.writeCommandData((const uint8_t[]){CMD_TEMP_SENSOR_WRITE, 0x05, 0x00}, 3); // < temp register
	EPD.writeCommandData((const uint8_t[]){CMD_DISP_UPDATE_CTRL2, 0xB1}, 2);	   // mode 1 (i2C)
	EPD.writeCommand(CMD_ACTIVATION);
	EPD.waitWhileBusy("_loadLUT", 5000);
	//Serial.println("ready. reading lut:");

	uint8_t *waveformbuffer = (uint8_t *)waveform;
	EPD.readData(CMD_READ_LUT, waveformbuffer, (SCREEN_LUT_LENGTH * 10) + 6);
	/*
		Serial.println("dump finished");

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
	*/
	display.powerOff();
	currentwaveform = *waveform;
	
}

void initEpd() {

	/*
	display.init();
	display.setFullWindow();
	display.setRotation(1);
	display.fillScreen(GxEPD_WHITE);
	display.setFont(&FreeSans9pt7b);
	display.setTextSize(1);
	display.setTextColor(GxEPD_RED);
	drawCentreString("TEST", 148, 50);

	display.setTextColor(GxEPD_BLACK);
	display.setCursor(20, 20);
	display.print("IP Address: ");
	display.print(WiFi.localIP());

	display.fillRect(150, 70, 30, 30, GxEPD_BLACK);
	display.drawRect(190, 70, 30, 30, GxEPD_BLACK);
	display.fillRect(230, 70, 30, 30, GxEPD_RED);

	displayWithProgress();
	*/
	display.powerOff();

}

void displayWithProgress() {
	;
	display.writeImage(display.getBlackBuffer(), display.getColorBuffer(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	EPD.writeCommand(0x22);
	EPD.writeData(0xf7);
	EPD.writeCommand(0x20);

	uint64_t waitmicros = micros();

#define CLK 13
#define DIO 15
	TM1637TinyDisplay6 led(CLK, DIO);
	led.setBrightness(BRIGHT_HIGH);
	led.clear();
	/*
	do {
		led.showNumber((long)(micros() - t) / 1000);
		yield();
	} while (EPD.isBusy());
	*/
	waitmicros += 5*20000;
	do {
	} while (micros() < waitmicros);
	const char *phasename[] = {"A", "B", "C", "D"};
	for (uint8_t wavegroup = 0; wavegroup < 7; wavegroup++) {
		led.showNumber(wavegroup, false, 1, 0);
		uint8_t repeat = currentwaveform.group[wavegroup].repeat + 1;
		for (uint8_t repetition = 0; repetition < repeat; repetition++) {
			led.showNumber(repeat-repetition, false, 3, 3);
			for (uint8_t phase = 0; phase < 4; phase++) {
				led.showString(phasename[phase], 1, 1, 0);
				uint8_t frames = currentwaveform.group[wavegroup].phaselength[phase];
				waitmicros += frames * 10000;
				do {
				} while (micros() < waitmicros);
			}
		}
	}

	display.powerOff();
}

void drawCentreString(const String &buf, int x, int y) {
	// void drawCentreString(const char *buf, int x, int y) {
	int16_t x1, y1;
	uint16_t w, h;
	display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); // calc width of new string
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

int checkEPDtask(int patternid) {
	if (patternid == 0) return 0;

	display.init();
	display.setFullWindow();
	display.setRotation(1);
	display.fillScreen(GxEPD_WHITE);

	switch (patternid) {
	case TEST_WHITE:
		break;

	case TEST_PATTERN1:
		display.fillRect(80, 0, 40, 40, GxEPD_BLACK);
		display.fillRect(124, 0, 40, 40, GxEPD_BLACK);
		display.fillRect(168, 0, 40, 40, GxEPD_BLACK);
		display.drawRect(80, 44, 40, 40, GxEPD_BLACK);
		display.drawRect(124, 44, 40, 40, GxEPD_BLACK);
		display.drawRect(168, 44, 40, 40, GxEPD_BLACK);
		display.fillRect(80, 88, 40, 40, GxEPD_RED);
		display.fillRect(124, 88, 40, 40, GxEPD_RED);
		display.fillRect(168, 88, 40, 40, GxEPD_RED);
		break;

	case TEST_PATTERN2:
		display.fillRect(80, 0, 40, 40, GxEPD_BLACK);
		display.drawRect(124, 0, 40, 40, GxEPD_BLACK);
		display.fillRect(168, 0, 40, 40, GxEPD_RED);
		display.fillRect(80, 44, 40, 40, GxEPD_BLACK);
		display.drawRect(124, 44, 40, 40, GxEPD_BLACK);
		display.fillRect(168, 44, 40, 40, GxEPD_RED);
		display.fillRect(80, 88, 40, 40, GxEPD_BLACK);
		display.drawRect(124, 88, 40, 40, GxEPD_BLACK);
		display.fillRect(168, 88, 40, 40, GxEPD_RED);
		break;

	case TEST_LOREMIPSUM:
		display.setCursor(0, 2);
		display.setTextColor(GxEPD_BLACK, GxEPD_WHITE);
		display.setTextSize(1);
		display.setFont();
		display.setTextWrap(true);
		display.print("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas quis velit libero. Praesent id turpis sapien. Integer aliquet varius tortor vitae feugiat. Donec sit amet varius velit. Maecenas aliquet lectus sit amet turpis auctor, quis maximus eros sagittis. Ut et rhoncus eros, sed dictum lorem. Donec urna dui, suscipit ut elit a.");
		display.setTextColor(GxEPD_RED, GxEPD_WHITE);
		display.print("Donec urna dui, suscipit ut elit a, hendrerit pellentesque urna. Nunc maximus, nunc et facilisis suscipit, mauris nisl volutpat mauris, vitae placerat odio metus vel dui. Ut tempus quis risus ut scelerisque. Mauris sagittis, turpis felis tincidunt nisl, vitae euismod metus. consectetur adipiscing elit. ");
		display.setTextColor(GxEPD_WHITE, GxEPD_BLACK);
		display.print("Donec urna dui, suscipit ut elit a, hendrerit pellentesque urna. Nunc maximus, nunc et facilisis suscipit, mauris nisl volutpat mauris, vitae placerat odio metus vel dui. Ut tempus quis risus ut scelerisque. Mauris sagittis, ante at dictum venenatis, turpis felis tincidunt nisl, vitae euismod metus. consectetur adipiscing elit. Maecenas quis velit libero.");
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

	displayWithProgress();

	display.powerOff();

	return 0;
}