#include <Wire.h>

#define NO_PINS 6
#define START_PIN 8

#define DEFAULT_I2C_SENSOR_ADDRESS  0x27
#define REGISTER_MAP_SIZE (NO_PINS*2)

#define FIRMWARE_VERSION "0.5.0"

#define MAX_WAIT 20000

#ifndef _DEBUG
    #define _DEBUG false
#endif

#ifndef _VDEBUG
    // verbose debug
    #define _VDEBUG false
#endif
