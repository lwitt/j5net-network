#ifndef MESSAGE_h
#define MESSAGE_h

#include <Arduino.h>
#include <JeeLib.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

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

// multi-part j2net elements

typedef struct{
	int temperature;
} PartTemp;

typedef struct{
	int pulseCount;
} PartPulseCount;

typedef struct{
	int temperature;
	int humidity;
} PartTempHumi;

typedef struct{
	byte id;
	byte value1;
	byte value2;
	char type;
} PartRelay;

typedef struct{
	byte code;
} PartDoorBell;

typedef struct{
	byte vcc;
} PartVcc;




class Message {
public:
	void clear();

	Message(byte source=0);

	void encode(byte parttype,void* part,byte partsize);
	void decode();

	void send(byte destination,byte powermode);

	byte getHeader();
	byte getSource();
	byte getSequence();
	byte getPayloadByte(byte);
	byte getPayloadSize();
	byte getTotalSize();

	void store(void* data,byte datasize);

	void sendSerial();

	void saveEncryptionKey(void* key);

	byte vccRead (byte count =4);

	private:
	struct {
	byte header;
	byte source;
	byte sequence;
	byte payload[64];
	} message;


	byte payloadSize;
};



#endif
