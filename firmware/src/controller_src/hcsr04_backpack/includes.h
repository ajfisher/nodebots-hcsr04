#include <avr/interrupt.h>

#if defined( __AVR_ATtiny85__ )
    #include <TinyWireS.h>
    #include <avr/power.h>

    #include <SendOnlySoftwareSerial.h>

    SendOnlySoftwareSerial Serial(3);
  
    #define PING_PIN 4
#else
    #include <Wire.h>
    #define PING_PIN 12
#endif

#ifndef _DEBUG
    #define _DEBUG false
#endif

#ifndef _VDEBUG
    #define _VDEBUG false
#endif
