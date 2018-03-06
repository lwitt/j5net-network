#ifndef MESSAGE_h
#define MESSAGE_h

#include <Arduino.h>
#ifndef ARDUINO_SAMD_ZERO
	#include <JeeLib.h>
	#include <avr/sleep.h>
#endif

#define J2NET_MASTER_ID 			1
#define J2NET_NETWORK_ID_1			77
#define J2NET_NETWORK_ID_0			76

#define J2NET_HEADER				77
#define J2NET_HEADER_SIZE			3

#define J2NET_PART_WELCOME			10
#define J2NET_PART_ALIVE			11
#define J2NET_PART_CAR_KNOCK			30
#define J2NET_PART_CAR_QUESTION		31
#define J2NET_PART_CAR_ANSWER			32
#define J2NET_PART_TEMP				50
#define J2NET_PART_PULSE_COUNT		51
#define J2NET_PART_TEMP_HUMI			52
#define J2NET_PART_NTP				53
#define J2NET_PART_RELAY_SET			54
#define J2NET_PART_RELAY_GET			55
//#define J2NET_PART_TEMP_HUMI_2		58
#define J2NET_PART_WEATHER			60
#define J2NET_PART_WEATHERX			61
#define J2NET_PART_HOME_STATUS_FAST		65
#define J2NET_PART_HOME_STATUS_SLOW		66
#define J2NET_PART_DOOR_BELL			68
#define J2NET_PART_VCC				70

#define RETRY_PERIOD    10
#define ACK_TIME        10

// multi-part j2net elements

typedef struct{
	int16_t temperature;
} PartTemp;

typedef struct{
	uint16_t pulseCount;
} PartPulseCount;

typedef struct{
	int16_t temperature;
	uint16_t humidity;
} PartTempHumi;

typedef struct{
	uint8_t id;
	uint8_t value1;
	uint8_t value2;
	char type;
} PartRelay;

typedef struct{
	uint8_t code;
} PartDoorBell;

typedef struct{
	uint8_t vcc;
} PartVcc;




class Message {
public:
	void clear();

	Message(uint8_t source=0);

	void encode(uint8_t parttype,void* part,uint8_t partsize);
	void decode();


	uint8_t getHeader();
	uint8_t getSource();
	uint8_t getSequence();
	uint8_t getPayloadByte(uint8_t);
	uint8_t getPayloadSize();
	uint8_t* getPayloadPtr();
	uint8_t getTotalSize();

	void store(void* data,uint8_t datasize);

	void sendSerial();

	#ifdef ARDUINO_SAMD_ZERO
	#else
	uint8_t waitForAck();
	bool send(uint8_t destination,uint8_t powermode,uint8_t retries,bool with_ack);
	#endif

	uint8_t vccRead (uint8_t count =4);
	uint8_t vccRead2 (uint8_t count =4);

private:
	struct {
		uint8_t header;
		uint8_t source;
		uint8_t sequence;
		uint8_t payload[64];
	} message;

	uint8_t payloadSize;
};



#endif
