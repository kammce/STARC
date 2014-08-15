#include "ASTARC.h"

void KeyValueSplit(char * msg, KeyValue_t * kvt) {
	char split[] = { US, 0 };
	int split_point = indexOf(msg, split);
	if(split_point == -1) {
		kvt->key = msg;
		kvt->value = kvt->empty;
	} else {
		kvt->key = msg;
		kvt->value = msg+split_point+1;
		msg[split_point] = 0;
	}
}

boolean strfind(char * haystack, char * needle) {
	if(strstr(haystack, needle) != 0) {
		return true;
	}
	return false;
}
int indexOf(char * haystack, char * needle) {
	char * start = needle;
	bool onTrail = true;
	int haylength = strlen(haystack)+1;
	for(int i=0; i < haylength; ++i) {
	    if(*needle == 0 && onTrail) {
	    	return (i-strlen(start));
	    } else if(haystack[i] == *needle) {
			++needle;
			onTrail = true;
		} else {
			needle = start;
			onTrail = false;
		}
	}
	return -1;
}
boolean iscontrolchar(char c)  {
 	return (c == '\n' || c == STX || c == ETX || c == ENQ || c == ACK || c == CMD || c == LOG || c == NACK || c == RS || c == US);
}
/*
class Encrypter
{
//private:
public:
	uint8_t keystream[STREAM_SIZE];
	uint8_t tempstream[STREAM_SIZE];
	char buff;
	uint16_t i,j,k;
	void crypt(char * msg);
	void generateNewStream(char * key);
};
void Encrypter::generateNewStream(char * key) {
	buff = 0; i = 0; j = 0; k = 0;

	int keylen = (int)strlen(key);

	uint8_t * S = keystream;
	uint8_t * T = tempstream;

	for(i = 0; i < STREAM_SIZE; ++i) {
		S[i] = i+ASCII_BYPASS;
		T[i] = key[(i+ASCII_BYPASS) % keylen];
	}
	for(i = 0; i < STREAM_SIZE; ++i) {
		j = (j + S[i] + T[i]) % (STREAM_SIZE);
		buff = S[i];
		S[i] = S[j];
		S[j] = buff;
	}
}
void Encrypter::crypt(char * msg) {
	i = 0; j = 0;
	uint8_t crypter = 0;
	uint8_t * S = keystream;
	uint8_t * T = tempstream;
	for(i = 0; i < STREAM_SIZE; ++i) {
		T[i] = S[i];
	}
	for(k = 0; msg[k] != 0; ++k) {
		i = (i + 1) % (STREAM_SIZE);
		j = (j + S[i]) % (STREAM_SIZE);
		buff = T[i];
		T[i] = T[j];
		T[j] = buff;
		crypter = T[(T[i] + T[j]) % (STREAM_SIZE)];
		msg[k] = msg[k] ^ crypter;
	}
}
*/

STARC::STARC(SoftwareSerial * serial, void (*function_ptr)(char *)) {
	read_pos = 0;
	read_start = false;
	for(int i = 0; i < 256; ++i) {
		read_buffer[i] = 0;
		write_buffer[i] = 0;
		tmp_buffer[i] = 0;
	}
	callback = function_ptr;
	blue = serial;
	blue->begin(9600);
	//encrypter.generateNewStream("TEST");
}
void STARC::send(char type, const char * fmt, ...) {
	if(fmt[0] == ACK && type == LOG) {
	    sprintf(write_buffer,"%c%c%c", STX, ACK, ETX);
	    Serial.print(write_buffer);
	    blue->print(write_buffer);
	    return;
	}
    va_list args;
    va_start(args,fmt);
    // Format tmp_buffer with arguments
    vsprintf(tmp_buffer,fmt,args);
    va_end(args);
    //encrypter.crypt(tmp_buffer);
    // Surround write_buffer with arguments
    sprintf(write_buffer,"%c%c%s%c", STX, type, tmp_buffer, ETX);
    blue->print(write_buffer);
    Serial.println(tmp_buffer);
    Serial.println(write_buffer);
}

void STARC::clearString() {
	for(int i = 0; i < 256; ++i) {
		read_buffer[i] = 0;
	}
	read_pos = 0;
}
uint8_t STARC::append(char c) {
	if(c == STX && read_start) {
		clearString();
		read_start = false;
		return 3;
	} else if(c == STX) {
		read_start = true;
		return 3;
	} else if(c == ETX && read_start) { // end of transmission in ASCII
		if(read_buffer[0] == ENQ) {
			Serial.println("ENQ");
			char ack[] = {ACK, 0};
			send(LOG, ack);
			return 4;
		}
		(*callback)(read_buffer);
		clearString();
		read_start = false;
		return 2;
	} else if(!isprint(c) && !iscontrolchar(c)) {
		return 0;
	}
	if(read_pos+1 > 255) { return 0; }
	read_buffer[read_pos++] = c;
	return 1;
}
boolean STARC::find(char * c_str) {
	if(strstr(read_buffer, c_str) != 0) {
		return true;
	}
	return false;
}
boolean STARC::update() {
	if (blue->available()) {
		char input = blue->read();
		append(input);
	}
}

boolean STARC::isConnected() {
	return connected;
};
void STARC::setCallback(void (*function_ptr)(char *)) {
	callback = function_ptr;
};