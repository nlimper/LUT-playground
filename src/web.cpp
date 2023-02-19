#include "web.h"
#include "GxEPDextra.h"
#include "epd.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void initWeb() {

	LittleFS.begin(true);

	server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");

	server.on("/getwaveform", HTTP_GET, [](AsyncWebServerRequest *request) {
		struct waveform waveformBuffer;
		struct waveform *waveform = &waveformBuffer;
		readLUT(waveform);
		request->send(200, "application/json", wave2json(waveform));
	});

	server.on("/pattern", HTTP_GET, [](AsyncWebServerRequest *request) {
		if (request->hasParam("id")) {
			runTask = request->getParam("id")->value().toInt();
		}
		request->send(200, "text/plain", "ok");
	});

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

	server.onNotFound(notFound);

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);

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

void wsRun(bool running) {
	StaticJsonDocument<500> doc;
	doc["running"] = running;
	ws.textAll(doc.as<String>());
}
