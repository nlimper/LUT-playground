#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

extern int runTask;

void initWeb();
void notFound(AsyncWebServerRequest *request);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void wsLog(String text);
void wsErr(String text);
