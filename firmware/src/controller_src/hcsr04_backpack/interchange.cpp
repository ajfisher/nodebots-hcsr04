#include "Arduino.h"
#include "interchange.h"

extern states state = BOOT;

void config_check() {
    // sees if the config pin is set and if do drop into config mode

    pinMode(CONFIG_PIN, INPUT);
    // delay for a moment to just let the pin stabilise
    delayMicroseconds(50);
    if (digitalRead(CONFIG_PIN) == HIGH) {
        state = CONFIG;
    } else {
        state = RUNNING;
    }
}
