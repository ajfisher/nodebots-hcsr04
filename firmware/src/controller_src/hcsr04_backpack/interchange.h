/** Interchange Library example from AJ Fisher **/

#ifndef INTERCHANGE_H
#define INTERCHANGE_H

#include <EEPROM.h>

/** EEPROM Memory map:

Byte    Type    Register
-------------------------
0-7     Null    Reserved
0x08    INT8    I2C_ADDRESS 
0x09    BOOL    Custom Address (1 = a custom address being used)
0x0A    INT8    Firmware ID from the repository
0x0B    INT8    Creator ID from the repository
    
**/
// EEPROM address locations to memory
#define INTERCHANGE_I2C_ADDRESS 0x08
#define INTERCHANGE_USE_CUSTOM 0x09
#define INTERCHANGE_FIRMWARE_ID 0x0A
#define INTERCHANGE_CREATOR_ID 0x0B

enum states {
    BOOT,
    CONFIG,
    RUNNING
};

extern states state;

// Pull this pin high at boot and the backpack will drop into config mode.
#define CONFIG_PIN 2

#define INTERCHANGE_VER_MAJ 0
#define INTERCHANGE_VER_MIN 1
#define INTERCHANGE_VER_PATCH 0
#define INTERCHANGE_VERSION "0.1.0"

void config_check(); // checks to see if config pin is gone high
void run_config(); // runs the config application.
void process_message(); // processes an incoming message.

#endif
