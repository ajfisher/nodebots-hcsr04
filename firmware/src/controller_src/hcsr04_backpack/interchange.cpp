#include "Arduino.h"
#include "interchange.h"

extern states state = BOOT;
Command commands[MAX_COMMANDS];
HardwareSerial *ser;

uint8_t current_command = 0;

void interchange_init() {
    // initialises the commands.
    attach_command("HELP", F("Prints this list. Try HELP <CMD> for more"), command_help);
    attach_command("DUMP", F("Prints out all of the information about this firmware"), command_dump);
    attach_command("I2C", F("Sets the I2C address and custom firmware flag.\n\n \
                eg: I2C 0x57 1 sets address 0x57 & custom flag bit"), command_set_i2c);
    attach_command("FID", F("Sets the firmware id. \n\n \
                eg: FID 0x02 sets the firmware id to 0x02"), command_set_firmware_id); 
    attach_command("CID", F("Sets the creator id. \n\n \
                eg: cID 0x09 sets the creator id to 0x09"), command_set_creator_id); 
}

void attach_command(char code[], String help, CommandFuncPtr cb) {
    // attaches the command to the command list.

    if (current_command < MAX_COMMANDS) {
        // add the command to the list
        Command c;
        strncpy(c.code, code, COMMAND_CODE_LENGTH);
        c.help = help;
        c.cmd = cb;
        commands[current_command] = c;
        current_command++;
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

void run_config(HardwareSerial serport) {
    // runs the config application
    
    ser = &serport;
    
    ser->begin(9600);
    ser->println(F("HCSR04 backpack firmware."));
    ser->print(F("Interchange version: "));
    ser->println(INTERCHANGE_VERSION);
    ser->println(F("\nEnter command followed by NL. H for help"));
    interchange_init();
}

void process_message() {
    // waits for the serial process to complete and then 

    // create a buffer
    // add chars to the buffer as it fills
    // when you get a \n then you're complete
    // process the message by pushing out to the other functions with appropriate
    // paramaters

}

void command_dump(String args) {
    // dumps the details of the firmware out.

    ser->println("Dumping data");
}

void command_set_i2c(String args) {
    // sets the I2C address of the firmware
    //
    ser->println("Setting the I2C address");
}

void command_set_firmware_id(String args) {
    // sets the firmware id

    ser->println("Setting the Firmware ID");
}

void command_set_creator_id(String args) {
    // sets the creator id

    ser->println("Setting the Creator ID");
}

void command_help(String args) {
    // provides help details.

    uint8_t cmd_index;
    if (args.length() >=2) {
        // we attempt to see if there is a command we should spit out instead.
        //cmd_index = command_item(args);
        if (cmd_index < 0) ser->println(F("Syntax error please use a command"));
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
        //ser->print(commands[cmd_index].code);
        ser->print(": ");
        //ser->println(commands[cmd_index].help);
    }


}


