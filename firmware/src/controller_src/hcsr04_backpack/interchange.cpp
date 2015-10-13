#include "Arduino.h"
#include "interchange.h"

extern states state = BOOT;
Command commands[MAX_COMMANDS];

uint8_t current_command = 0;

void interchange_init() {
    // initialises the library.
    attach_command("HELP", "Print out the help list", command_help);
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
        Serial.println("MAX COMMANDS exceeded");
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

void run_config() {
    // runs the config application
    Serial.begin(9600);
    Serial.println(F("HCSR04 backpack firmware."));
    Serial.print(F("Interchange version: "));
    Serial.println(INTERCHANGE_VERSION);
    Serial.println(F("\nEnter command followed by NL. H for help"));
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

void command_help(String args) {
    Serial.println("This is help");
}


