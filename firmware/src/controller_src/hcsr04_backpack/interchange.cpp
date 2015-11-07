#include "Arduino.h"
#include "interchange.h"

extern states state = BOOT;
Command commands[MAX_COMMANDS]; // list of commands
Stream* ser; // mapping to a serial port
String command; // current command being worked on.

uint8_t mem_addresses[] = {INTERCHANGE_I2C_ADDRESS, INTERCHANGE_USE_CUSTOM, INTERCHANGE_FIRMWARE_ID, INTERCHANGE_CREATOR_ID};
uint8_t address_len = sizeof(mem_addresses); 

byte firmware_id = 0x0;
byte creator_id = 0x0;
bool custom_i2c_addr = false;
byte i2c_address = 0x0;
String firmware_version = "";

uint8_t current_commands = 0;

void interchange_init(String fw_ver) {
    // initialises the commands.
    attach_command("HELP", F("Prints this list. Try HELP <CMD> for more"), command_help);
    attach_command("CLR", F("Clears the eeprom settings"), command_clear_eeprom);
    attach_command("DUMP", F("Prints out all of the information about this firmware"), command_dump);
    attach_command("I2C", F("Sets the I2C address and custom firmware flag.\n\n \
                eg: I2C 0x57 1 sets address 0x57 & custom flag bit"), command_set_i2c);
    attach_command("FID", F("Sets the firmware id. \n\n \
                eg: FID 0x02 sets the firmware id to 0x02"), command_set_firmware_id); 
    attach_command("CID", F("Sets the creator id. \n\n \
                eg: cID 0x09 sets the creator id to 0x09"), command_set_creator_id); 

    initialise_vars(fw_ver);
}

void initialise_vars(String fw_ver) {
    // just initialises the variables from EEPROM etc.

    if (EEPROM.read(INTERCHANGE_CREATOR_ID) != INTERCHANGE_EEPROM_DEFAULT) {
        creator_id = EEPROM.read(INTERCHANGE_CREATOR_ID);
    }

    if (EEPROM.read(INTERCHANGE_FIRMWARE_ID) != INTERCHANGE_EEPROM_DEFAULT) {
        firmware_id = EEPROM.read(INTERCHANGE_FIRMWARE_ID);
    }

    if (EEPROM.read(INTERCHANGE_USE_CUSTOM) != INTERCHANGE_EEPROM_DEFAULT) {
        custom_i2c_addr = (bool)EEPROM.read(INTERCHANGE_USE_CUSTOM);
    }

    if (EEPROM.read(INTERCHANGE_I2C_ADDRESS) != INTERCHANGE_EEPROM_DEFAULT) {
        i2c_address = EEPROM.read(INTERCHANGE_I2C_ADDRESS);
    }

    firmware_version = fw_ver;
}

void attach_command(char code[], String help, CommandFuncPtr cb) {
    // attaches the command to the command list.

    if (current_commands < MAX_COMMANDS) {
        // add the command to the list
        Command c;
        strncpy(c.code, code, COMMAND_CODE_LENGTH);
        c.help = help;
        c.cmd = cb;
        commands[current_commands] = c;
        current_commands++;
    } else {
        // throw error and don't do it. 
        ser->println("MAX COMMANDS exceeded");
    }
}

void config_check() {
    // sees if the config pin is set and if so drop into config mode

    pinMode(CONFIG_PIN, INPUT);
    // delay for a moment to just let the pin stabilise
    delayMicroseconds(50);
    if (digitalRead(CONFIG_PIN) == HIGH) {
        state = CONFIG;
    } else {
        state = RUNNING;
    }
}

void run_config(Stream& serport, String fw_ver) {
    // runs the config application
    
    ser = &serport; // assign the stream to the serial port

    ser->print(F("Interchange ver: "));
    ser->println(INTERCHANGE_VERSION);
    ser->println(F("\nEnter command, followed by NL. Type HELP for more."));
    ser->println(F(">>"));
    interchange_init(fw_ver);
}


bool use_custom_address() {
    // returns whether we should use the custom address or not
    
    if (EEPROM.read(INTERCHANGE_USE_CUSTOM) != INTERCHANGE_EEPROM_DEFAULT) {
        return ((bool)EEPROM.read(INTERCHANGE_USE_CUSTOM));
    }
    return (false);
}
    
uint8_t get_i2c_address() {
    // returns the address from eeprom if set

    if (EEPROM.read(INTERCHANGE_I2C_ADDRESS) != INTERCHANGE_EEPROM_DEFAULT) {
        return((uint8_t) EEPROM.read(INTERCHANGE_I2C_ADDRESS));
    }
    return (0x0);
}

/** DEFINE INTERCHANGE INTERACTIVE COMMANDS HERE **/

void interchange_commands() {
    // this is effectively the "main" loop of the interchange config system.

    while (ser->available() > 0) {
        char ch = ser->read();
        if (ch == 10) {
            // new line so now we can attempt to process the line
            process_command(command);
            command = "";
        } else if ((ch < 10 && ch > 0) || (ch > 10 && ch < 32)) {
          // ignore control chars up to space and allow nulls to pass through
         ; 
        } else {
            command += String(ch);
        }
    }
}

void process_command(String command) {
    // waits for the serial process to complete and then 

    String argv[2]; // two tokens, the command and the params.

    int8_t split = command.indexOf(' ');
    argv[0] = command.substring(0, split);
    if (split > 0) {
        argv[1] = command.substring(split+1);
    }
    int8_t cmd_index = command_item(argv[0]);
    if (cmd_index >= 0) {
        commands[cmd_index].cmd(argv[1]);
    } else {
        ser->println("Invalid command");
    }
}

int command_item(String cmd_code) {
    // this method does all of the comparison stuff to determine the id of a command
    // which it then passes back

    uint8_t i=0;
    boolean arg_found = false;
    // look through the array of commands until you find it or else you exhaust the list.
    while (!arg_found && i<current_commands) {
        if (cmd_code.equalsIgnoreCase((String)commands[i].code)) {
            arg_found = true;
        } else {
            i++;
        }
    }

    if (arg_found) {
        return (i);
    } else {
        return (-1);
    }
}

void command_dump(String args) {
    // dumps the details of the firmware out.

    ser->print(F("{"));
    
    ser->print(F("\"firmwareID\":"));
    if (firmware_id == 0x0) {
        ser->print(F("\"undefined\""));
    } else {
        ser->print(firmware_id);
    }
    ser->print(F(","));

    ser->print(F("\"creatorID\":"));
    if (creator_id == 0x0) {
        ser->print(F("\"undefined\""));
    } else {
        ser->print(creator_id);
    }
    ser->print(F(","));

    ser->print(F("\"use_custom_addr\":"));
    if (custom_i2c_addr) {
        ser->print(F("true"));
    } else {
        ser->print(F("false"));
    }
    ser->print(F(","));

    ser->print(F("\"i2c_address\":"));
    ser->print(i2c_address);
    ser->print(F(","));

    ser->print(F("\"ic_version\":\""));
    ser->print(INTERCHANGE_VERSION);
    ser->print(F("\","));

    ser->print(F("\"fw_version\":\""));
    ser->print(firmware_version);
    ser->print(F("\","));

    ser->print(F("\"compile_date\":\""));
    ser->print(__DATE__ " " __TIME__);
    ser->print(F("\""));

    ser->println(F("}"));
}

void command_clear_eeprom(String args) {
    // sets the EEPROM address back to default
    ser->println("Clearing the EEPROM addresses");

    for (uint8_t i = 0; i < address_len; i++) {
        EEPROM.write(mem_addresses[i], INTERCHANGE_EEPROM_DEFAULT);
    }
    initialise_vars(firmware_version);

    ser->println(F("OK EEPROM cleared"));
}

void command_set_i2c(String args) {
    // sets the I2C address of the firmware
    // This address is 0x01 -> 0xFE so only need to read the
    // one byte off the args list.
    if (args.length() <= 0) {
        ser->println(F("ERR No address supplied"));
        return;
    }

    // now we need to get the I2C address and potentially whether it's
    // a custom address or not.

    String argv[2]; // up to two tokens, the address and custom bit.
    
    int8_t split = args.indexOf(' ');
    argv[0] = args.substring(0, split);
    if (split > 0) {
        argv[1] = args.substring(split+1);
    } else {
        argv[1] = "0";
    }

    char buf[10];
    argv[0].toCharArray(buf, 10);
    i2c_address = (byte)atoi(buf);

    EEPROM.write(INTERCHANGE_I2C_ADDRESS, i2c_address);
    ser->print(F("OK I2C address set "));
    ser->println(i2c_address, HEX);

    // set the custom I2C byte if needed.
    // we can check if the third byte is a 1 or 0 as a char.
    // and use that to set the custom address or not.
    if (argv[1][0] == '1') {
        custom_i2c_addr = true;
    } else {
        custom_i2c_addr = false;
    }

    EEPROM.write(INTERCHANGE_USE_CUSTOM, custom_i2c_addr);
}

void command_set_firmware_id(String args) {
    // sets the firmware id

    if (args.length() <= 0) {
        ser->println(F("ERR No firmware ID supplied"));
        return;
    }
    
    char buf[10];
    args.toCharArray(buf, 10);
    firmware_id = atoi(buf);
    EEPROM.write(INTERCHANGE_FIRMWARE_ID, firmware_id);
    ser->print(F("OK firmware ID set "));
    ser->println(firmware_id, HEX);
}

void command_set_creator_id(String args) {
    // sets the creator id

    if (args.length() <= 0) {
        ser->println(F("ERR No creator ID supplied"));
        return;
    }
    
    char buf[10];
    args.toCharArray(buf, 10);
    creator_id = atoi(buf);
    EEPROM.write(INTERCHANGE_CREATOR_ID, creator_id);
    ser->print(F("OK creator ID set "));
    ser->println(creator_id, HEX);
}

void command_help(String args) {
    // provides help details.

    int8_t cmd_index;
    if (args.length() >=2) {
        // we attempt to see if there is a command we should spit out instead.
        cmd_index = command_item(args);
        if (cmd_index < 0) ser->println(F("Syntax error please provide a command"));
    } else {
        cmd_index = -1;
    }

    if (cmd_index < 0) {
        ser->println(F("Interchange configuration help"));
        ser->print(F("Interchange version: "));
        ser->println(INTERCHANGE_VERSION);
        ser->println(F("Try help <cmd> for more info about each command"));
        ser->println(F("Available commands"));
        ser->println(F("------------------"));
        for (int i=0; i<MAX_COMMANDS; i++){
            if (commands[i].help != ""){
                ser->print(commands[i].code);
                if (i % 3 == 0) {
                    ser->println();
                } else {
                    ser->print("\t");
                }
            }
        }
        ser->println();
    } else {
        ser->println(F("HELP"));
        ser->print(commands[cmd_index].code);
        ser->print(": ");
        ser->println(commands[cmd_index].help);
    }
}
