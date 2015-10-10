/** Interchange Library example from AJ Fisher **/

#ifndef INTERCHANGE_H
#define INTERCHANGE_H

#include <EEPROM.h>

/** EEPROM Memory map:

Byte    Type    Register
-------------------------
0-7     Null    Reserved
8       INT8    I2C_ADDRESS 
9       BOOL    Custom Address (1 = a custom address being used)
10      INT8    Device ID from the 
    

**/
enum states {
    BOOT,
    CONFIG,
    RUNNING
};

extern states state;

// Pull this pin high at boot and the backpack will drop into config mode.
#define CONFIG_PIN 2

#define INTERCHANGE_VERSION "0.1.0"


void config_check();


#endif
