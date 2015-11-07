#include <Wire.h>

#define DEFAULT_I2C_SENSOR_ADDRESS  0x27
#define REGISTER_MAP_SIZE 2

#define FIRMWARE_VERSION "0.3.1"

#define PING_PIN 8

#ifndef _DEBUG
    #define _DEBUG false
#endif

#ifndef _VDEBUG
    // verbose debug
    #define _VDEBUG false
#endif
