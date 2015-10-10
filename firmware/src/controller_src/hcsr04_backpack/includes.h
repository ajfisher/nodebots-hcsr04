#include <Wire.h>

#define DEFAULT_I2C_SENSOR_ADDRESS  0x27
#define REGISTER_MAP_SIZE 2

#define PING_PIN 8

#ifndef _DEBUG
    #define _DEBUG false
#endif

#ifndef _VDEBUG
    #define _VDEBUG false
#endif
