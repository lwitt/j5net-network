#ifndef RELAY_h
#define RELAY_h

#include <Arduino.h>
#include <JeeLib.h>
#include <avr/sleep.h>
#include <EEPROM.h>
#include <SoftPWM.h>

#define 	MAXRELAY 		8
// #define     WITH_SERIAL       1

typedef struct{
      byte id;
      byte value1;
      byte value2;
      byte pin;
      char mode;
} RelayState;

class Relay {
public:
      Relay(RelayState[]);

      void showCurrentState();
      void loadCurrentState();
      void updateCurrentState(byte id,byte value1, byte value2, char mode);
      void setPinValue(byte index);
      void setPinMode(byte index);
      void initEEPROM();
      void setState (RelayState);

private:
      RelayState currentState[MAXRELAY];
};

#endif
