#include <Arduino.h>
#include <WiFi.h>
#include "pass.h"
#include "web.h"
#include "epd.h"
#include "7segment.h"

//const char *ssid = "*****";
//const char *password = "*****";

int runTask = 0;

void setup() {

	Serial.begin(115200);

	initLED();

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.printf("WiFi Failed!\n");
		return;
	}

	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());

	initWeb();
	initEpd();
}

void loop() {
	runTask = checkEPDtask(runTask);

	vTaskDelay(100 / portTICK_PERIOD_MS);
}
