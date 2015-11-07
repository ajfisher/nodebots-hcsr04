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

#define INTERCHANGE_EEPROM_DEFAULT 0xFF

enum states {
    BOOT,
    CONFIG,
    RUNNING
};

extern states state;

// Pull this pin high at boot and the backpack will drop into config mode.
#define CONFIG_PIN 2

#define INTERCHANGE_VER_MAJ 0
#define INTERCHANGE_VER_MIN 2
#define INTERCHANGE_VER_PATCH 0
#define INTERCHANGE_VERSION "0.2.0"

void interchange_init(String fw_ver); // initialises the interchange lib.
void initialise_vars(String fw_ver); // used to init the vars used across
void attach_command(char code[], String help, CommandFuncPtr cb); 
void config_check(); // checks to see if config pin is gone high
void run_config(Stream& serport, String fw_ver); // runs the config application.
void interchange_commands(); // the main loop that processes the commands.
bool use_custom_address(); // returns whether we should use the custom address or not
uint8_t get_i2c_address(); // returns the address from eeprom if set


// command related methods
int command_item(String cmd_code); // given a string, find the related command
void process_command(String command); // processes an incoming message.

// default commands
void command_help(String args);
void command_clear_eeprom(String args);
void command_dump(String args);
void command_set_i2c(String args);
void command_set_firmware_id(String args);
void command_set_creator_id(String args);

#endif
