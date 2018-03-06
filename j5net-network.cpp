#include "j5net-network.h"

volatile bool adcDone;

#ifdef ARDUINO_SAMD_ZERO
#else
// for low-noise/-power ADC readouts, we'll use ADC completion interrupts
ISR(ADC_vect) { adcDone = true; }
#endif

Message::Message(uint8_ source){
	clear();
	message.source = source;
	message.sequence = 0;
}

void Message::clear() {
	message.header = 0;
	//message.sequence = 0;
	payloadSize = 0;
	memset(message.payload,0,64);
}

void Message::encode(uint8_ parttype,void* part,uint8_ partsize)
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



#ifdef ARDUINO_SAMD_ZERO
#else

uint8_ Message::waitForAck() {
	MilliTimer ackTimer;
	while (!ackTimer.poll(ACK_TIME)) {
		if (rf12_recvDone() && rf12_crc == 0 &&
		// see http://talk.jeelabs.net/topic/811#post-4712
		rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | message.source))
		return 1;
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}
	return 0;
}

bool Message::send(uint8_ destination,uint8_ powermode,uint8_ retries,bool with_ack)
{
	if (powermode>0) rf12_sleep(RF12_WAKEUP);
	for (uint8_ j = 0; j < retries; j++) {
		int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
		rf12_sendStart(RF12_HDR_ACK | RF12_HDR_DST | destination, &message, 3+payloadSize);
		//rf12_sendNow(RF12_HDR_ACK | RF12_HDR_DST | destination, &message, 3+payloadSize);
		switch (powermode) {
			case 0: // standard mode
			rf12_sendWait(0);
			break;
			case 1: // low-power mode
			rf12_sendWait(2);
			break;
			case 2: //ultra low-power mode
			rf12_sendWait(3);
			break;
		}
		clear();

		if (with_ack) {
			uint8_ acked = waitForAck();
			if (acked) {
				#if DEBUG
				Serial.print("acked! (");
				Serial.print((int) j);
				Serial.println(")");
				Serial.flush();
				#endif
				return (true);
			}
		}

		if (powermode>0) rf12_sleep(RF12_SLEEP);
		delay(RETRY_PERIOD * 100);
	}
	return(false);
}
#endif

uint8_ Message::getHeader() {
	return message.header;
}

uint8_ Message::getSource() {
	return message.source;
}

uint8_ Message::getSequence() {
	return message.sequence;
}

uint8_ Message::getPayloadByte(uint8_ pos) {
	return message.payload[pos];
}

uint8_ Message::getPayloadSize() {
	return payloadSize;
}

uint8_* Message::getPayloadPtr() {
	return &(message.payload[0]);
}

uint8_ Message::getTotalSize() {
	return payloadSize+3;
}

void Message::store(void* data,uint8_ datasize) {
	memcpy(&message,data,datasize);
	if (datasize>=3)
	payloadSize=datasize-3;
	else
	payloadSize=0;
}

void Message::sendSerial() {
	#if defined(__AVR_ATtiny84__)
	#else
	Serial.print("MSG:");
	Serial.print(getHeader()); 	Serial.print(' ');
	Serial.print(getSource());	Serial.print(' ');
	Serial.print(getSequence());	Serial.print(' ');

	for (uint8_ i = 0; i < getPayloadSize(); i++) {
		Serial.print(getPayloadByte(i));
		Serial.print(' ');
	}

	Serial.println(); Serial.flush(); delay(1);
	#endif
}


uint8_ Message::vccRead (uint8_ count) {
	#ifdef ARDUINO_SAMD_ZERO
	return(115); // 3.3V
	#else
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
	#endif
}

// improved version supporting Moteino..

uint8_t Message::vccRead2(bool restoreMux) {
  unsigned long result;
  uint8_ saveADMUX;

  saveADMUX = ADMUX;
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284P__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  #if defined(__AVR_ATmega2560__)
        ADCSRB = 0;
  #endif

  delay(20); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  result = (high<<8) | low;
  result = (55U * 1024U) / (result + 1) - 50; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000 (because...= 3300*1023/3 since 1.1 is exactly 1/3 of 3.3V)

  if (restoreMux) ADMUX = saveADMUX;

  return result; // Vcc in millivolts
}
