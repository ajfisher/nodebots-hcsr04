#include "includes.h"
#include <avr/interrupt.h>

#include "interchange.h"

#if _VDEBUG
    #define PING_FREQUENCY 150 // go slow so we can see what is happening
#else
    #define PING_FREQUENCY 25 // go more or less as fast as possible, assuming wait out of 20ms
#endif

byte register_map[REGISTER_MAP_SIZE];

byte current_pin = START_PIN;
byte current_duration_index = 0;

volatile int32_t durations[NO_PINS];

volatile int32_t duration; // duration of the ping in us
int32_t last_ping_time = 0; // last time the ping occurred in ms
int32_t ping_freq = PING_FREQUENCY;
volatile int32_t ping_emit_time = 0;  // time the ping was emitted in us

volatile bool pinging = false; // used to determine when we're actually pinging
volatile bool timedout = false; // used to see if we timed out or not

// Interrupt vector for external interrupt on pin PCINT7..0
// This will be called when any of the pins D8 - D13 on an Arduino Uno change.

// the ping pin will flip HIGH at the point when the pulse has completed
// and timing should begin, it will then flip LOW once the sound wave is received
// so we need to detect both of these states

ISR(PCINT0_vect) {

    if (digitalRead(current_pin) == HIGH) {
        // pulse is initiated so record it
        pinging = true;
        ping_emit_time = micros();
        timedout = false;
    } else {
        // pulse is coming back
        durations[current_duration_index] = micros() - ping_emit_time;
        timedout = false;
        pinging = false;
    }
}

// Interrupt that fires once a ms and does a quick check to see if the
// ping has timed out. If it has, set the appropriate values and turn off
// the port change interrupt so it doesn't come back at a later point.

SIGNAL(TIMER0_COMPA_vect) {

    if (pinging && (micros() - ping_emit_time) > MAX_WAIT) {
        timedout = true;
        pinging = false;
        disablePCInterrupt();
        durations[current_duration_index] = MAX_WAIT;
    } else {
        timedout = false;
    }
}

void setup() {

    // check to see if we're in config mode
    config_check();

    if (state == CONFIG) {
        Serial.begin(9600);
        run_config(Serial, FIRMWARE_VERSION);

    } else if (state == RUNNING) {
        uint8_t i2c_address;

        if (use_custom_address() ) {
            if (get_i2c_address() > 0) {
                i2c_address = get_i2c_address();
            } else {
                i2c_address = DEFAULT_I2C_SENSOR_ADDRESS;
            }
        } else {
            i2c_address = DEFAULT_I2C_SENSOR_ADDRESS;
        }

        Wire.begin(i2c_address);
        Wire.onRequest(requestData);

        // enable timer interrupt to determine timeouts
        OCR0A = 0xAF;
        TIMSK0 |= _BV(OCIE0A);

    }

#if _DEBUG
    if (state != CONFIG) {
        Serial.begin(9600);
    }

    Serial.println(F("HCSR04 FIRMWARE - DEBUG MODE"));
    Serial.print("Use custom: ");
    Serial.println(use_custom_address());
    Serial.print("I2C address: ");
    Serial.println(get_i2c_address());
#endif
}


void loop() {

    if (state == RUNNING) {
        get_distance();
        delay(5);
        if (timedout) {
            timedout = false;
        }
    } else if (state == CONFIG) {
        interchange_commands();
    }
}

void disablePCInterrupt() {
    // disable all interrupts temporarily
    cli();

    // disable pin change interrupt vector
    //PCMSK0 &= ~_BV(PCINT0);
    PCMSK0 &= ~digitalPinToBitMask(current_pin);

    // clear pin change interrupt flag register
    PCIFR &= ~_BV(PCIF2);
    // re-enable all interrupts
    sei();
}

void enablePCInterrupt() {
    // disable all interrupts temporarily
    cli();

    // set pin change mask for interrupt on port B
    // specifically enabling pin change interrupt on current pin.
    PCMSK0 |= digitalPinToBitMask(current_pin);//_BV(PCINT0);
    // enable Pin Change Interrupt control register for the PCMSK0
    PCICR |= _BV(PCIE0);
    // re-enable all interrupts
    sei();
}


void get_distance() {

    if ((millis() - last_ping_time) > ping_freq) {

        // disable interrupt while pinMode is OUTPUT
        // not sure if this is actually necessary, but just
        // playing it safe to avoid false interrupt when
        // pin mode is OUTPUT
        disablePCInterrupt();

    #if _VDEBUG
        Serial.print("P: ");
        Serial.print(current_pin);
        Serial.print(" D: ");
        Serial.print(durations[current_duration_index]);
        Serial.print(" CM:");
        Serial.print((long)durations[current_duration_index] / 29 / 2);
        if (durations[current_duration_index] >= MAX_WAIT) {
            Serial.print(" TO");
        }
        Serial.println();
    #endif

        // set old pin into a known state.
        pinMode(current_pin, OUTPUT);

        // move to the next pin and then set up for a new pulse
        current_duration_index++;
        current_pin++;
        // cycle the values around if needed
        if (current_duration_index >= NO_PINS) {
            current_duration_index = 0;
            current_pin = START_PIN;
        }

        // reset these as we're about to start a new set of pulses
        // on the sensor
        pinging = false;
        timedout = false;

        pinMode(current_pin, OUTPUT);
        digitalWrite(current_pin, LOW);
        delayMicroseconds(2);
        digitalWrite(current_pin, HIGH);
        delayMicroseconds(5);
        digitalWrite(current_pin, LOW);

        pinMode(current_pin, INPUT);

        durations[current_duration_index] = 0;
        // we'll use a pin change interrupt to notify when the
        // ping pin changes rather than being blocked by
        // Arduino's built-in pulseIn function
        // Additionally, if the timer detects a timeout condition then it will
        // resolve as well.
        enablePCInterrupt();

        last_ping_time = millis();
    }
}

void requestData() {

    for (uint8_t i = 0; i < NO_PINS; i++) {
        register_map[i*2] = durations[i] >> 8; // msb
        register_map[i*2 + 1] = durations[i] & 0xFF; //LSB
    }

    // ATMEGA cat write out a buffer
    Wire.write(register_map, REGISTER_MAP_SIZE);

#if _DEBUG
    Serial.print("rm: ");
    Serial.print(register_map[0]);
    Serial.print(" ");
    Serial.print(register_map[1]);
    Serial.println();
#endif
}
