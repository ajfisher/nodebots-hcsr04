#include "Arduino.h"
#include "interchange.h"

extern states state = BOOT;
Command commands[MAX_COMMANDS]; // list of commands
Stream* ser; // mapping to a serial port
String command; // current command being worked on.

uint8_t current_commands = 0;

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

void run_config(Stream& serport) {
    // runs the config application
    
    ser = &serport; // assign the stream to the serial port

    ser->println(F("HCSR04 backpack firmware."));
    ser->print(F("Interchange version: "));
    ser->println(INTERCHANGE_VERSION);
    ser->println(F("\nEnter command, followed by NL. Type HELP for more."));
    interchange_init();
}

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

    // create a buffer
    // add chars to the buffer as it fills
    // when you get a \n then you're complete
    // process the message by pushing out to the other functions with appropriate
    // paramaters

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


