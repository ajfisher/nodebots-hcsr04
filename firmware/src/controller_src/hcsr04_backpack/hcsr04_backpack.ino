#include "includes.h"
#include <avr/interrupt.h>

#include "interchange.h"

#if _VDEBUG
    #define PING_FREQUENCY 1000 // milliseconds between pings
#else
    #define PING_FREQUENCY 100 // go much slower so we can see wtf is going on
#endif

byte register_map[REGISTER_MAP_SIZE];

volatile int32_t duration; // duration of the ping
int32_t last_ping_time = 0; // last time the ping occurred in ms
int32_t ping_freq = PING_FREQUENCY;
volatile int32_t ping_emit_time = 0;  // time the ping was emitted in us

volatile bool pinging = false; // used to determine when we're pinging

// Interrupt vector for external interrupt on pin PCINT7..0
// This will be called when any of the pins D8 - D13 on an Arduino Uno change.

// the ping pin will flip HIGH at the point when the pulse has completed
// and timing should begin, it will then flip LOW once the sound wave is received
// so we need to detect both of these states

ISR(PCINT0_vect) {
    
    if (digitalRead(PING_PIN) == HIGH) {
        pinging = true;
        ping_emit_time = micros();
    } else { 
        duration = micros() - ping_emit_time;
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
        delay(20);
    } else if (state == CONFIG) {
        interchange_commands();
    }
}

void disablePCInterrupt() {
    // disable all interrupts temporarily
    cli();

    // disable pin change interrupt vector
    PCMSK0 &= ~_BV(PCINT0);

    // clear pin change interrupt flag register
    PCIFR &= ~_BV(PCIF2);
    // re-enable all interrupts
    sei();
}

void enablePCInterrupt() {
    // disable all interrupts temporarily
    cli();

    // enable pin change interrupt on D8 
    PCMSK0 |= _BV(PCINT0);

    // enable pin change interrupt 0
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
        //Serial.print("D: ");
        //Serial.println(duration);
        Serial.print("CM:");
        Serial.println((long)duration / 29 / 2);
    #endif
    
        pinging = false;

        pinMode(PING_PIN, OUTPUT);
        digitalWrite(PING_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(PING_PIN, HIGH);
        delayMicroseconds(5);
        digitalWrite(PING_PIN, LOW);

        pinMode(PING_PIN, INPUT);

        // we'll use a pin change interrupt to notify when the
        // ping pin changes rather than being blocked by
        // Arduino's built-in pulseIn function
        enablePCInterrupt();

        last_ping_time = millis();
    }
}

void requestData() {

    register_map[0] = duration >> 8; // msb
    register_map[1] = duration & 0xFF; //LSB

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
