#ifndef ASTARC_H
#define ASTARC_H

#include "Arduino.h"
#include "SoftwareSerial.h"

#define ASCII_BYPASS 32
#define STREAM_SIZE (256-ASCII_BYPASS)

#define STX		0x02 // Start of Transmission
#define ETX		0x03 // End of Transmission
#define ENQ		0x05 // Enquiry
#define ACK		0x06 // Acknowledge
#define CMD		0x12 // Send/Recieve CMD
#define LOG		0x14 // LOG message
#define NACK	0x15 // Negative Ack
#define RS 		0x1E // Record Seperator
#define US		0x1F // Unit Seperator

typedef struct {
	char * key;
	char * value;
	char empty[2];
} KeyValue_t;

void KeyValueSplit(char * msg, KeyValue_t * kvt);
boolean strfind(char * haystack, char * needle);
int indexOf(char * haystack, char * needle);
boolean iscontrolchar(char c);

class STARC
{
private:
	// Communication Object
	SoftwareSerial * blue;
	char tmp_buffer[256];
	char write_buffer[256];
	char read_buffer[255];
	uint8_t read_pos;
	boolean connected;
	boolean read_start;
	// Callback Function Pointer
	void (*callback)(char *);
public:
	STARC(SoftwareSerial * serial, void (*function_ptr)(char *));
	boolean append(char c);
	boolean find(char * c_str);
	void clearString();
	void send(char type, const char * fmt, ...);
	boolean isConnected();
	boolean setCallback();
	boolean update();
	void setCallback(void (*function_ptr)(char *));
};

#endif