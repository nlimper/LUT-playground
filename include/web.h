#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void initWeb();
void notFound(AsyncWebServerRequest *request);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void wsLog(String text);
void wsErr(String text);
void wsWaveform(String jsonstring);
