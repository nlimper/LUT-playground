#include "web.h"
#include "GxEPDextra.h"
#include "epd.h"
#include <Arduino.h>

#define DYNAMIC_JSON_DOCUMENT_SIZE 5000
#include <ArduinoJson.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include "taskqueue.h"
#include "AsyncJson.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
extern QueueHandle_t taskQueue;

void initWeb() {

	LittleFS.begin(true);

	server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");

	server.on("/getwaveform", HTTP_GET, [](AsyncWebServerRequest *request) {
		int8_t temperature = 20;
		if (request->hasParam("temp")) {
			temperature = request->getParam("temp")->value().toInt();
		}
		queueMessage msg = {TASK_READLUT, temperature};
		xQueueSend(taskQueue, &msg, portMAX_DELAY);
		//request->send(200, "application/json", wave2json(waveform));
		request->send(200, "text/plain", "ok");
	});

	server.on("/pattern", HTTP_GET, [](AsyncWebServerRequest *request) {
		if (request->hasParam("id")) {
			queueMessage msg = {TASK_PATTERN, request->getParam("id")->value().toInt() };
			xQueueSend(taskQueue, &msg, portMAX_DELAY);
		}
		request->send(200, "text/plain", "ok");
	});

	server.on("/run", HTTP_POST, [](AsyncWebServerRequest *request) {
		if (request->hasParam("cmd",true)) {
			queueMessage msg = {TASK_RUN, request->getParam("cmd", true)->value().toInt()};
			xQueueSend(taskQueue, &msg, portMAX_DELAY);
		}
		request->send(200, "text/plain", "ok");
	});

/*
	// Send a GET request to <IP>/get?message=<message>
	server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
		String message;
		if (request->hasParam("message")) {
			message = request->getParam("message")->value();
		} else {
			message = "No message sent";
		}
		request->send(200, "text/plain", "Hello, GET: " + message);
	});

	// Send a POST request to <IP>/post with a form field message set to <message>
	server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
		String message;
		if (request->hasParam("message", true)) {
			message = request->getParam("message", true)->value();
		} else {
			message = "No message sent";
		}
		request->send(200, "text/plain", "Hello, POST: " + message);
	});
*/

	server.onNotFound(notFound);

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);

	AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/putwaveform", [](AsyncWebServerRequest *request, JsonVariant &json) {
		const JsonObject &jsonObj = json.as<JsonObject>();
		waveform my_waveform = json2wave(jsonObj);
		useLut(&my_waveform);
		if (jsonObj["cmd"]) {
			queueMessage msg = {TASK_RUN, jsonObj["cmd"]};
			xQueueSend(taskQueue, &msg, portMAX_DELAY);
		} else {
			queueMessage msg = {TASK_RUNSOLO, jsonObj["solo"]};
			xQueueSend(taskQueue, &msg, portMAX_DELAY);
		}
		request->send(200, "text/plain", "ok");
	});
	server.addHandler(handler);

	server.begin();
}

void notFound(AsyncWebServerRequest *request) {
	request->send(404, "text/plain", "Not found");
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {

	if (type == WS_EVT_CONNECT) {

		Serial.println("Websocket client connection received");
		// client->text("Hello from ESP32 Server");

	} else if (type == WS_EVT_DISCONNECT) {
		Serial.println("Client disconnected");
	}
}

void wsLog(String text) {
	StaticJsonDocument<500> doc;
	doc["logMsg"] = text;
	ws.textAll(doc.as<String>());
}

void wsErr(String text) {
	StaticJsonDocument<500> doc;
	doc["errMsg"] = text;
	ws.textAll(doc.as<String>());
}

void wsWaveform(String jsonstring) {
	ws.textAll(jsonstring);
}
