#define I2C_SENSOR_ADDRESS  0x27
#define REGISTER_MAP_SIZE 2

#include "includes.h"

#if _VDEBUG
    #define PING_FREQUENCY 1000 // milliseconds between pings
#else
    #define PING_FREQUENCY 100 // go much slower so we can see wtf is going on
#endif

byte register_map[REGISTER_MAP_SIZE];

volatile int32_t duration; // duration of the ping
int32_t last_ping_time = 0; // last time the ping occurred in ms
int32_t ping_freq = PING_FREQUENCY;
int32_t ping_emit_time = 0;  // time the ping was emitted in us

bool pinging = false; // used to determine when we're pinging


// Interrupt vector for external interrupt on pin PCINT7..0
// This will be called when any of the pins D0 - D4 on the trinket change
// or pins D8 - D13 on an Arduino Uno change.

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

#if defined( __AVR_ATtiny85__ )
    // Set prescaler so CPU runs at 16MHz
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);

    // Use TinyWire for ATTINY
    TinyWireS.begin(I2C_SENSOR_ADDRESS);
    TinyWireS.onRequest(requestData);

#else
    Wire.begin(I2C_SENSOR_ADDRESS);
    Wire.onRequest(requestData);
#endif

#if _DEBUG
    Serial.begin(9600);
    Serial.println("Ping Sensor I2C");
#endif
}


void loop() {

    get_distance();

#if defined( __AVR_ATtiny85__ )
    // USE this for ATTINY as you can't use delay
    TinyWireS_stop_check();
#else
    delay(20);
#endif
}

void disablePCInterrupt() {
    // disable all interrupts temporarily
    cli();

    // disable pin change interrupt
    PCMSK &= ~_BV(PCINT4);

    // clear pin change interrupt flag register
#if defined( __AVR_ATtiny85__ )
    GIFR &= ~_BV(PCIF);
#else
    PCIFR &= ~_BV(PCIF2);
#endif
    // re-enable all interrupts
    sei();
}

void enablePCInterrupt() {
    // disable all interrupts temporarily
    cli();

    // enable pin change interrupt on PB4 (D4 on Trinket, D12 on Uno)
    PCMSK |= _BV(PCINT4);

    // enable pin change interrupt 0
#if defined( __AVR_ATtiny85__ )
    GIMSK |= _BV(PCIE);
#else
    PCICR |= _BV(PCIE0);
#endif
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

#if defined( __AVR_ATtiny85__ )
    // ATTINY you need to do a send of each register individually.
    TinyWireS.send(register_map[0]);
    TinyWireS.send(register_map[1]);
#else
    // ATMEGA cat write out a buffer
    Wire.write(register_map, REGISTER_MAP_SIZE);
#endif

#if _DEBUG
    Serial.print("rm: ");
    Serial.print(register_map[0]);
    Serial.print(" ");
    Serial.print(register_map[1]);
    Serial.println();
#endif
}
