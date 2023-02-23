#include <Arduino.h>

struct queueMessage {
	uint8_t taskCmd;
	uint8_t param;
	String data;
};

#define TASK_PATTERN 1
#define TASK_RUN 2
#define TASK_RUNSOLO 3
#define TASK_READLUT 4

void runTask(void *parameter);
