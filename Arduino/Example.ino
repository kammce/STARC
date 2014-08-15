#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>
#include "SoftwareSerial.h"
#include "ASTARC.h"

void serverFeedback(char * msg);

SoftwareSerial SoftSerial(10, 11); // RX, TX

typedef struct {
	uint8_t hour;
	uint8_t minute;
} Time;

boolean state = LOW;
int counter = 25;
char buffer[255] = {0};
char large_buffer[512] = {0};

Time current_time = {0,0};
Time arm_time     = {0,0};
Time disarm_time  = {0,0};

STARC com(&SoftSerial, &serverFeedback);

void setup()
{
	Serial.begin(9600);
	Serial.print("Good Bye World\n");

	pinMode(13, OUTPUT);
	pinMode(9, INPUT);
	pinMode(8, INPUT);
	pinMode(7, INPUT);

	digitalWrite(13, LOW);
}

void loop()
{
	if(digitalRead(9) == HIGH) {
		//counter+=5;
		com.send(CMD, "GetTime");
		delay(300);
		if(state == HIGH) {
			state = LOW;
		} else if(state == LOW) {
			state = HIGH;
		}
		digitalWrite(13, state);
	}
	if(digitalRead(8) == HIGH) {
		//counter-=5;
		com.send(CMD, "FoundPerson");
		delay(150);
		if(state == HIGH) {
			state = LOW;
		} else if(state == LOW) {
			state = HIGH;
		}
		digitalWrite(13, state);
	}
	if(digitalRead(7) == HIGH) {
		com.send(CMD, "Shutdown");
		delay(150);
		if(state == HIGH) {
			state = LOW;
		} else if(state == LOW) {
			state = HIGH;
		}
		digitalWrite(13, state);
	}
	com.update();
}

void serverFeedback(char * msg) {
	KeyValue_t kv = {0,0, ""};
	char tmp[256] = {0};
	strcpy(tmp, msg);
	KeyValueSplit(tmp, &kv);
	if(strcmp(kv.key, "CurrentTime") == 0) {
		Serial.println(kv.value);
		sscanf(kv.value, "%2hhu:%2hhu", &current_time.hour, &current_time.minute);
		sprintf(buffer, "Current Hour %2hhu, Current Minute %2hhu\n",current_time.hour, current_time.minute);
		com.send(LOG, buffer);
	} else if(strcmp(kv.key, "ArmTime") == 0) {
		sscanf(kv.value, "%2hhu:%2hhu", &arm_time.hour, &arm_time.minute);
		sprintf(buffer, "Arm Hour %2hhu, Arm Minute %2hhu\n", arm_time.hour, arm_time.minute);
		com.send(LOG, buffer);
	} else if(strcmp(kv.key, "DisarmTime") == 0) {
		sscanf(kv.value, "%2hhu:%2hhu", &disarm_time.hour, &disarm_time.minute);
		sprintf(buffer, "Arm Hour %2hhu, Arm Minute %2hhu\n", disarm_time.hour, disarm_time.minute);
		com.send(LOG, buffer);
	} else if(strcmp(kv.key, "GetCurrentTime") == 0) {
		com.send(LOG, "EmbeddedCurrentTime::%2hhu:%2hhu", current_time.hour, current_time.minute);
	} else {
		com.send(LOG, "INVALID\n");
	}
}