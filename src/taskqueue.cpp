#include "taskqueue.h"
#include "epd.h"
#include <Arduino.h>
#include "web.h"

QueueHandle_t taskQueue;
extern struct waveform currentwaveform;

void runTask(void *parameter) {
	queueMessage msg;
	while (1) {
		if (xQueueReceive(taskQueue, &msg, portMAX_DELAY) == pdPASS) {
			Serial.print("Received item from queue: id=");
			Serial.println(msg.taskCmd);
			switch (msg.taskCmd) {

				case TASK_PATTERN:
					sendPattern(msg.param);
					break;

				case TASK_RUN:
					displayWithProgress(msg.param);
					break;

				case TASK_RUNSOLO:
					displaySolo(msg.param);
					break;

				case TASK_READLUT:
					int8_t temperature = msg.param;
					struct waveform *waveform = &currentwaveform;
					readLUT(waveform, temperature);
					wsWaveform(wave2json(waveform));
					break;
					
				}
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}