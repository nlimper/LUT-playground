#include <Arduino.h>
#include <WiFi.h>
#include "pass.h"
#include "web.h"
#include "epd.h"
#include "7segment.h"
#include "taskqueue.h"

//const char *ssid = "*****";
//const char *password = "*****";

extern QueueHandle_t taskQueue;
extern TM1637TinyDisplay6 led;

void setup() {

	Serial.begin(115200);
	taskQueue = xQueueCreate(10, sizeof(queueMessage));

	initLED();
	led.showString("wifi..");

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.printf("WiFi Failed!\n");
		return;
	}

	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());

	led.showString("web...");
	initWeb();
	led.showString("epd...");
	initEpd();
	led.showString("ready");

	xTaskCreate(runTask, "Taskqueue", 10000, NULL, 2, NULL);
}

void loop() {
	vTaskDelay(100 / portTICK_PERIOD_MS);
}
