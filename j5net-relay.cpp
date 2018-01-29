#include "j5net-relay.h"

SOFTPWM_DEFINE_EXTERN_OBJECT_WITH_PWM_LEVELS(20, 100);

Relay::Relay(RelayState init[]){
	for (byte i=0;i<MAXRELAY;i++) {
		currentState[i]=init[i];
	}
}

void Relay::updateCurrentState(byte id,byte value1,byte value2,char mode) {
	bool found = false; byte i=0;
	// byte nbitems = sizeof(currentState) / sizeof(RelayState);
	while (!found && i<MAXRELAY){
		if (currentState[i].id==id) {
			found = true;

			if ((currentState[i].value1 != value1) || (currentState[i].value2 != value2)) {
				currentState[i].value1 = value1;
				currentState[i].value2 = value2;
				// EEPROM.put(0,currentState);
				// Serial.println("eeprom updated!");
				setPinValue(i);
			}

			if (currentState[i].mode!=mode && (mode=="a" || mode=="d")) {
				currentState[i].mode = mode;
				setPinMode(i);
				EEPROM.put(0,currentState);
				#ifdef WITH_SERIAL
				Serial.println("eeprom updated!");
				#endif
			}
		}
		i++;
	}
}

void Relay::loadCurrentState() {
	EEPROM.get(0,currentState);

	for (byte i=0;i<MAXRELAY;i++) {
		// TODO set to output
		setPinMode(i);
		setPinValue(i);
	}
}

void Relay::initEEPROM() {
	EEPROM.put(0,currentState);
}

void Relay::setPinValue(byte index) {
	char buffer [60];
	char mode[10];

	switch (currentState[index].mode) {
		case 'd':
			strcpy(mode,"digital");
			if (currentState[index].value1 == 255)
				digitalWrite(currentState[index].pin, HIGH);
			if (currentState[index].value1 == 0)
				digitalWrite(currentState[index].pin, LOW);
		break;
		case 'a':
			strcpy(mode,"analog");
			Palatis::SoftPWM.set(currentState[index].pin,currentState[index].value1);
		break;
		case 'f':
			strcpy(mode,"fade");
		break;
		case 'p':
			strcpy(mode,"pulse");
			if (currentState[index].value2 > 0)
				digitalWrite(currentState[index].pin, HIGH);
			else
				digitalWrite(currentState[index].pin, LOW);
		break;

		default :
		strcpy(mode,"unknown");
		break;
	}

	sprintf(buffer,"set pin %d to value %d/%d (%s)",currentState[index].pin,currentState[index].value1,currentState[index].value2,mode);
	#ifdef WITH_SERIAL
	Serial.println(buffer);
	#endif
}


void Relay::setPinMode(byte index) {
	char buffer [60];
	char mode[10];

	switch (currentState[index].mode) {
		case 'd':
		strcpy(mode,"digital");
		break;
		case 'a':
		strcpy(mode,"analog");
		break;
		case 'f':
		strcpy(mode,"fade");
		break;
		case 'p':
		strcpy(mode,"pulse");
		break;
		default :
		strcpy(mode,"unknown");
		break;
	}
	#ifdef WITH_SERIAL
	sprintf(buffer,"set pin %d to %s mode",currentState[index].pin,mode);
	Serial.println(buffer);
	#endif
	pinMode(currentState[index].pin,OUTPUT);
	//TO DO : the real state as to be defined !!
}

bool Relay::eventLoop() {
	byte i=0; bool found=false;
	while (i<MAXRELAY) {
		if (currentState[i].mode == 'p' && currentState[i].value2>0) {
			found = true;
			currentState[i].value2--;

			if (currentState[i].value2==0) {
				setPinValue(i);
			}
		}
		i++;
	}
	return(found);
}

void Relay::showCurrentState() {
	for (byte i=0;i<MAXRELAY;i++) {
		Serial.print(currentState[i].id);
		Serial.print(":");
		Serial.print(currentState[i].value1);
		Serial.print(":");
		Serial.print(currentState[i].value2);
		Serial.print(":");
		Serial.print(currentState[i].pin);
		Serial.print(":");
		Serial.print(currentState[i].mode);
		Serial.println();

	}
}
