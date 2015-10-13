/** Interchange Library example from AJ Fisher **/

#ifndef INTERCHANGE_H
#define INTERCHANGE_H

// set up the command objectsi
#define COMMAND_CODE_LENGTH 5
typedef void (*CommandFuncPtr)(String args); // typedef to the command

struct Command {
   char code[COMMAND_CODE_LENGTH]; // the code used to call the command
   String help; // the snippet used for help
   CommandFuncPtr cmd; // pointer to the command to be called
};

#define MAX_COMMANDS 10

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

void interchange_init(); // initialises the interchange lib.
void attach_command(char code[], String help, CommandFuncPtr cb); 
void config_check(); // checks to see if config pin is gone high
void run_config(); // runs the config application.
void process_message(); // processes an incoming message.

// default commands
void command_help(String);

#endif
