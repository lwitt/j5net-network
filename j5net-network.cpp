#include "j5net-network.h"

volatile bool adcDone;

// for low-noise/-power ADC readouts, we'll use ADC completion interrupts
ISR(ADC_vect) { adcDone = true; }

Message::Message(byte source){
	 	clear();
		message.source = source;
}

void Message::clear() {
	message.header = 0;
	//message.sequence = 0;
	payloadSize = 0;
	memset(message.payload,0,64);
}

void Message::encode(byte parttype,void* part,byte partsize)
{
    if (message.header==0) {
          message.header=J2NET_HEADER;
          message.sequence++;
    }

   message.payload[payloadSize]=parttype;
   memcpy(&message.payload[payloadSize+1],part,partsize);
   payloadSize+=partsize+1;

   // for (int i=0;i<cursor;i++){
 // 	   Serial.print("[");
 //   		Serial.print(payload[i]);
 // 		Serial.print("] ");
 // 	}
 //   Serial.println();
}

void Message::decode() {
	// TO BE DONE
}

void Message::send(byte destination,byte powermode)
{
    if (powermode>0) rf12_sleep(RF12_WAKEUP);
	int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}

	// TODO (MEDIUM) : length of header is const (3)
	rf12_sendStart(RF12_HDR_ACK | RF12_HDR_DST | destination, &message, 3+payloadSize);
	switch (powermode) {
		case 0: // standard mode
			rf12_sendWait(0);
			break;
		case 1: // low-power mode
			rf12_sendWait(2);
			rf12_sleep(RF12_SLEEP);
			break;
		case 2: //ultra low-power mode
			rf12_sendWait(3);
			rf12_sleep(RF12_SLEEP);
			break;
	}

	clear();
}

byte Message::getHeader() {
	return message.header;
}

byte Message::getSource() {
	return message.source;
}

byte Message::getSequence() {
	return message.sequence;
}

byte Message::getPayloadByte(byte pos) {
	return message.payload[pos];
}

byte Message::getPayloadSize() {
	return payloadSize;
}

byte Message::getTotalSize() {
	return payloadSize+3;
}

void Message::store(void* data,byte datasize) {
	memcpy(&message,data,datasize);
	if (datasize>=3)
		payloadSize=datasize-3;
	else
		payloadSize=0;
}

void Message::sendSerial() {
	Serial.print("MSG:");
	Serial.print(getHeader()); 	Serial.print(' ');
	Serial.print(getSource());	Serial.print(' ');
	Serial.print(getSequence());	Serial.print(' ');

	for (byte i = 0; i < getPayloadSize(); i++) {
		Serial.print(getPayloadByte(i));
	  	Serial.print(' ');
  	}

	Serial.println(); Serial.flush(); delay(1);
}

void Message::saveEncryptionKey(void* key) {
	//byte numkey[16];
	//memcpy((void*) key, numkey, 16);

    eeprom_write_block(key, RF12_EEPROM_EKEY, 16);
    //rf12_encrypt(RF12_EEPROM_EKEY);
}

byte Message::vccRead (byte count) {
  set_sleep_mode(SLEEP_MODE_ADC);
  // use VCC as AREF and internal bandgap as input
#if defined(__AVR_ATtiny84__)
  ADMUX = 33;
#else
  ADMUX = bit(REFS0) | 14;
#endif
  bitSet(ADCSRA, ADIE);
  while (count-- > 0) {
    adcDone = false;
    while (!adcDone)
      sleep_mode();
  }
  bitClear(ADCSRA, ADIE);
  // convert ADC readings to fit in one byte, i.e. 20 mV steps:
  //  1.0V = 0, 1.8V = 40, 3.3V = 115, 5.0V = 200, 6.0V = 250
  return (55U * 1024U) / (ADC + 1) - 50;
}
