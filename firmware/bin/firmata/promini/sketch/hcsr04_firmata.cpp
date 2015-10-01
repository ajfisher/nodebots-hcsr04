#line 1 "hcsr04_firmata.ino"
  
                                                                        
                                                                
                                      
  
                                                                          
                                                     
  
                                   
   

  
                                                                       
                                                                
                                                             
                                                           

                                                               
                                                            
                                                              
                                                                    

                                                                   

                                                    
  

  
                                                                
   

#include <Servo.h>
#include <Wire.h>
#include "./Firmata.h"

                                           
#define I2C_WRITE B00000000
#define I2C_READ B00001000
#define I2C_READ_CONTINUOUSLY B00010000
#define I2C_STOP_READING B00011000
#define I2C_READ_WRITE_MODE_MASK B00011000
#define I2C_10BIT_ADDRESS_MODE_MASK B00100000

#define MAX_QUERIES 8
#define MINIMUM_SAMPLING_INTERVAL 10

#define REGISTER_NOT_SPECIFIED -1

#define PING_READ                0x75                           

                                                                                
                   
                                                                                

                   
#include "Arduino.h"
void attachServo(byte pin, int minPulse, int maxPulse);
void detachServo(byte pin);
void readAndReportData(byte address, int theRegister, byte numBytes);
void outputPort(byte portNumber, byte portValue, byte forceSend);
void checkDigitalInputs(void);
void setPinModeCallback(byte pin, int mode);
void analogWriteCallback(byte pin, int value);
void digitalWriteCallback(byte port, int value);
void reportAnalogCallback(byte analogPin, int value);
void reportDigitalCallback(byte port, int value);
void sysexCallback(byte command, byte argc, byte *argv);
void enableI2CPins();
void disableI2CPins();
void systemResetCallback();
void setup();
void loop();
#line 56
int analogInputsToReport = 0;                                        

                         
byte reportPINs[TOTAL_PORTS];                                           
byte previousPINs[TOTAL_PORTS];                            

                        
byte pinConfig[TOTAL_PINS];                                      
byte portConfigInputs[TOTAL_PORTS];                                                 
int pinState[TOTAL_PINS];                                             

                     
unsigned long currentMillis;                                                
unsigned long previousMillis;                                           
unsigned int samplingInterval = 19;                                                   

              
struct i2c_device_info {
  byte addr;
  int reg;
  byte bytes;
};

                                  
i2c_device_info query[MAX_QUERIES];

byte i2cRxData[32];
boolean isI2CEnabled = false;
signed char queryIndex = -1;
unsigned int i2cReadDelayTime = 0;                                                                       

Servo servos[MAX_SERVOS];
byte servoPinMap[TOTAL_PINS];
byte detachedServos[MAX_SERVOS];
byte detachedServoCount = 0;
byte servoCount = 0;


                                                                                
            
                                                                                

void attachServo(byte pin, int minPulse, int maxPulse)
{
  if (servoCount < MAX_SERVOS) {
                                                                       
    if (detachedServoCount > 0) {
      servoPinMap[pin] = detachedServos[detachedServoCount - 1];
      if (detachedServoCount > 0) detachedServoCount--;
    } else {
      servoPinMap[pin] = servoCount;
      servoCount++;
    }
    if (minPulse > 0 && maxPulse > 0) {
      servos[servoPinMap[pin]].attach(PIN_TO_DIGITAL(pin), minPulse, maxPulse);
    } else {
      servos[servoPinMap[pin]].attach(PIN_TO_DIGITAL(pin));
    }
  } else {
    Firmata.sendString("Max servos attached");
  }
}

void detachServo(byte pin)
{
  servos[servoPinMap[pin]].detach();
                                                           
                                                    
  if (servoPinMap[pin] == servoCount && servoCount > 0) {
    servoCount--;
  } else if (servoCount > 0) {
                                                                           
                                                       
    detachedServoCount++;
    detachedServos[detachedServoCount - 1] = servoPinMap[pin];
  }

  servoPinMap[pin] = 255;
}

void readAndReportData(byte address, int theRegister, byte numBytes) {
                                                          
                                                                                   
                                                                                          
  if (theRegister != REGISTER_NOT_SPECIFIED) {
    Wire.beginTransmission(address);
#if ARDUINO >= 100
    Wire.write((byte)theRegister);
#else
    Wire.send((byte)theRegister);
#endif
    Wire.endTransmission();
                              
    if (i2cReadDelayTime > 0) {
                                                                
      delayMicroseconds(i2cReadDelayTime);
    }
  } else {
    theRegister = 0;                                         
  }

  Wire.requestFrom(address, numBytes);                                          

                                                                    
  if (numBytes < Wire.available()) {
    Firmata.sendString("I2C Read Error: Too many bytes received");
  } else if (numBytes > Wire.available()) {
    Firmata.sendString("I2C Read Error: Too few bytes received");
  }

  i2cRxData[0] = address;
  i2cRxData[1] = theRegister;

  for (int i = 0; i < numBytes && Wire.available(); i++) {
#if ARDUINO >= 100
    i2cRxData[2 + i] = Wire.read();
#else
    i2cRxData[2 + i] = Wire.receive();
#endif
  }

                                                    
  Firmata.sendSysex(SYSEX_I2C_REPLY, numBytes + 2, i2cRxData);
}

void outputPort(byte portNumber, byte portValue, byte forceSend)
{
                                                      
  portValue = portValue & portConfigInputs[portNumber];
                                                             
  if (forceSend || previousPINs[portNumber] != portValue) {
    Firmata.sendDigitalPort(portNumber, portValue);
    previousPINs[portNumber] = portValue;
  }
}

                                                                                
                                                                               
                                                     
void checkDigitalInputs(void)
{
                                                                       
                                                                    
                                                 
  if (TOTAL_PORTS > 0 && reportPINs[0]) outputPort(0, readPort(0, portConfigInputs[0]), false);
  if (TOTAL_PORTS > 1 && reportPINs[1]) outputPort(1, readPort(1, portConfigInputs[1]), false);
  if (TOTAL_PORTS > 2 && reportPINs[2]) outputPort(2, readPort(2, portConfigInputs[2]), false);
  if (TOTAL_PORTS > 3 && reportPINs[3]) outputPort(3, readPort(3, portConfigInputs[3]), false);
  if (TOTAL_PORTS > 4 && reportPINs[4]) outputPort(4, readPort(4, portConfigInputs[4]), false);
  if (TOTAL_PORTS > 5 && reportPINs[5]) outputPort(5, readPort(5, portConfigInputs[5]), false);
  if (TOTAL_PORTS > 6 && reportPINs[6]) outputPort(6, readPort(6, portConfigInputs[6]), false);
  if (TOTAL_PORTS > 7 && reportPINs[7]) outputPort(7, readPort(7, portConfigInputs[7]), false);
  if (TOTAL_PORTS > 8 && reportPINs[8]) outputPort(8, readPort(8, portConfigInputs[8]), false);
  if (TOTAL_PORTS > 9 && reportPINs[9]) outputPort(9, readPort(9, portConfigInputs[9]), false);
  if (TOTAL_PORTS > 10 && reportPINs[10]) outputPort(10, readPort(10, portConfigInputs[10]), false);
  if (TOTAL_PORTS > 11 && reportPINs[11]) outputPort(11, readPort(11, portConfigInputs[11]), false);
  if (TOTAL_PORTS > 12 && reportPINs[12]) outputPort(12, readPort(12, portConfigInputs[12]), false);
  if (TOTAL_PORTS > 13 && reportPINs[13]) outputPort(13, readPort(13, portConfigInputs[13]), false);
  if (TOTAL_PORTS > 14 && reportPINs[14]) outputPort(14, readPort(14, portConfigInputs[14]), false);
  if (TOTAL_PORTS > 15 && reportPINs[15]) outputPort(15, readPort(15, portConfigInputs[15]), false);
}

                                                                                
                                                                           
                                                       
   
void setPinModeCallback(byte pin, int mode)
{
  if (pinConfig[pin] == I2C && isI2CEnabled && mode != I2C) {
                                                          
                                                                       
    disableI2CPins();
  }
  if (IS_PIN_DIGITAL(pin) && mode != SERVO) {
    if (servoPinMap[pin] < MAX_SERVOS && servos[servoPinMap[pin]].attached()) {
      detachServo(pin);
    }
  }
  if (IS_PIN_ANALOG(pin)) {
    reportAnalogCallback(PIN_TO_ANALOG(pin), mode == ANALOG ? 1 : 0);                         
  }
  if (IS_PIN_DIGITAL(pin)) {
    if (mode == INPUT) {
      portConfigInputs[pin / 8] |= (1 << (pin & 7));
    } else {
      portConfigInputs[pin / 8] &= ~(1 << (pin & 7));
    }
  }
  pinState[pin] = 0;
  switch (mode) {
    case ANALOG:
      if (IS_PIN_ANALOG(pin)) {
        if (IS_PIN_DIGITAL(pin)) {
          pinMode(PIN_TO_DIGITAL(pin), INPUT);                         
          digitalWrite(PIN_TO_DIGITAL(pin), LOW);                             
        }
        pinConfig[pin] = ANALOG;
      }
      break;
    case INPUT:
      if (IS_PIN_DIGITAL(pin)) {
        pinMode(PIN_TO_DIGITAL(pin), INPUT);                         
        digitalWrite(PIN_TO_DIGITAL(pin), LOW);                             
        pinConfig[pin] = INPUT;
      }
      break;
    case OUTPUT:
      if (IS_PIN_DIGITAL(pin)) {
        digitalWrite(PIN_TO_DIGITAL(pin), LOW);               
        pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
        pinConfig[pin] = OUTPUT;
      }
      break;
    case PWM:
      if (IS_PIN_PWM(pin)) {
        pinMode(PIN_TO_PWM(pin), OUTPUT);
        analogWrite(PIN_TO_PWM(pin), 0);
        pinConfig[pin] = PWM;
      }
      break;
    case SERVO:
      if (IS_PIN_DIGITAL(pin)) {
        pinConfig[pin] = SERVO;
        if (servoPinMap[pin] == 255 || !servos[servoPinMap[pin]].attached()) {
                                                                           
                             
          attachServo(pin, -1, -1);
        }
      }
      break;
    case I2C:
      if (IS_PIN_I2C(pin)) {
                              
                                                                   
        pinConfig[pin] = I2C;
      }
      break;
    default:
      Firmata.sendString("Unknown pin mode");                                  
  }
                                                 
}

void analogWriteCallback(byte pin, int value)
{
  if (pin < TOTAL_PINS) {
    switch (pinConfig[pin]) {
      case SERVO:
        if (IS_PIN_DIGITAL(pin))
          servos[servoPinMap[pin]].write(value);
        pinState[pin] = value;
        break;
      case PWM:
        if (IS_PIN_PWM(pin))
          analogWrite(PIN_TO_PWM(pin), value);
        pinState[pin] = value;
        break;
    }
  }
}

void digitalWriteCallback(byte port, int value)
{
  byte pin, lastPin, mask = 1, pinWriteMask = 0;

  if (port < TOTAL_PORTS) {
                                                                
    lastPin = port * 8 + 8;
    if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
    for (pin = port * 8; pin < lastPin; pin++) {
                                                      
      if (IS_PIN_DIGITAL(pin)) {
                                                          
                                                                 
        if (pinConfig[pin] == OUTPUT || pinConfig[pin] == INPUT) {
          pinWriteMask |= mask;
          pinState[pin] = ((byte)value & mask) ? 1 : 0;
        }
      }
      mask = mask << 1;
    }
    writePort(port, (byte)value, pinWriteMask);
  }
}


                                                                                
                                                                          
   
                                                                  
   
void reportAnalogCallback(byte analogPin, int value)
{
  if (analogPin < TOTAL_ANALOG_PINS) {
    if (value == 0) {
      analogInputsToReport = analogInputsToReport &~ (1 << analogPin);
    } else {
      analogInputsToReport = analogInputsToReport | (1 << analogPin);
                                                                       
                                                                     
                      
      Firmata.sendAnalog(analogPin, analogRead(analogPin));
    }
  }
                                                 
}

void reportDigitalCallback(byte port, int value)
{
  if (port < TOTAL_PORTS) {
    reportPINs[port] = (byte)value;
                                                                      
                                                                   
                    
    if (value) outputPort(port, readPort(port, portConfigInputs[port]), true);
  }
                                                                   
                                                                     
                                                                   
                                                                
                                                                          
                              
}

                                                                                
                       
                                                                                

void sysexCallback(byte command, byte argc, byte *argv)
{
  byte mode;
  byte slaveAddress;
  byte data;
  int slaveRegister;
  unsigned int delayTime;

  switch (command) {
    case I2C_REQUEST:
      mode = argv[1] & I2C_READ_WRITE_MODE_MASK;
      if (argv[1] & I2C_10BIT_ADDRESS_MODE_MASK) {
        Firmata.sendString("10-bit addressing not supported");
        return;
      }
      else {
        slaveAddress = argv[0];
      }

      switch (mode) {
        case I2C_WRITE:
          Wire.beginTransmission(slaveAddress);
          for (byte i = 2; i < argc; i += 2) {
            data = argv[i] + (argv[i + 1] << 7);
#if ARDUINO >= 100
            Wire.write(data);
#else
            Wire.send(data);
#endif
          }
          Wire.endTransmission();
          delayMicroseconds(70);
          break;
        case I2C_READ:
          if (argc == 6) {
                                            
            slaveRegister = argv[2] + (argv[3] << 7);
            data = argv[4] + (argv[5] << 7);                  
          }
          else {
                                                
            slaveRegister = REGISTER_NOT_SPECIFIED;
            data = argv[2] + (argv[3] << 7);                  
          }
          readAndReportData(slaveAddress, (int)slaveRegister, data);
          break;
        case I2C_READ_CONTINUOUSLY:
          if ((queryIndex + 1) >= MAX_QUERIES) {
                                            
            Firmata.sendString("too many queries");
            break;
          }
          if (argc == 6) {
                                            
            slaveRegister = argv[2] + (argv[3] << 7);
            data = argv[4] + (argv[5] << 7);                  
          }
          else {
                                                
            slaveRegister = (int)REGISTER_NOT_SPECIFIED;
            data = argv[2] + (argv[3] << 7);                  
          }
          queryIndex++;
          query[queryIndex].addr = slaveAddress;
          query[queryIndex].reg = slaveRegister;
          query[queryIndex].bytes = data;
          break;
        case I2C_STOP_READING:
          byte queryIndexToSkip;
                                                                              
                                                      
          if (queryIndex <= 0) {
            queryIndex = -1;
          } else {
                                                                       
                                                                               
                                                                      
            for (byte i = 0; i < queryIndex + 1; i++) {
              if (query[i].addr == slaveAddress) {
                queryIndexToSkip = i;
                break;
              }
            }

            for (byte i = queryIndexToSkip; i < queryIndex + 1; i++) {
              if (i < MAX_QUERIES) {
                query[i].addr = query[i + 1].addr;
                query[i].reg = query[i + 1].reg;
                query[i].bytes = query[i + 1].bytes;
              }
            }
            queryIndex--;
          }
          break;
        default:
          break;
      }
      break;
    case I2C_CONFIG:
      delayTime = (argv[0] + (argv[1] << 7));

      if (delayTime > 0) {
        i2cReadDelayTime = delayTime;
      }

      if (!isI2CEnabled) {
        enableI2CPins();
      }

      break;
    case SERVO_CONFIG:
      if (argc > 4) {
                                                                                  
        byte pin = argv[0];
        int minPulse = argv[1] + (argv[2] << 7);
        int maxPulse = argv[3] + (argv[4] << 7);

        if (IS_PIN_DIGITAL(pin)) {
          if (servoPinMap[pin] < MAX_SERVOS && servos[servoPinMap[pin]].attached()) {
            detachServo(pin);
          }
          attachServo(pin, minPulse, maxPulse);
          setPinModeCallback(pin, SERVO);
        }
      }
      break;
    case SAMPLING_INTERVAL:
      if (argc > 1) {
        samplingInterval = argv[0] + (argv[1] << 7);
        if (samplingInterval < MINIMUM_SAMPLING_INTERVAL) {
          samplingInterval = MINIMUM_SAMPLING_INTERVAL;
        }
      } else {
                                                
      }
      break;
    case EXTENDED_ANALOG:
      if (argc > 1) {
        int val = argv[1];
        if (argc > 2) val |= (argv[2] << 7);
        if (argc > 3) val |= (argv[3] << 14);
        analogWriteCallback(argv[0], val);
      }
      break;
    case CAPABILITY_QUERY:
      Firmata.write(START_SYSEX);
      Firmata.write(CAPABILITY_RESPONSE);
      for (byte pin = 0; pin < TOTAL_PINS; pin++) {
        if (IS_PIN_DIGITAL(pin)) {
          Firmata.write((byte)INPUT);
          Firmata.write(1);
          Firmata.write((byte)OUTPUT);
          Firmata.write(1);
        }
        if (IS_PIN_ANALOG(pin)) {
          Firmata.write(ANALOG);
          Firmata.write(10);
        }
        if (IS_PIN_PWM(pin)) {
          Firmata.write(PWM);
          Firmata.write(8);
        }
        if (IS_PIN_DIGITAL(pin)) {
          Firmata.write(SERVO);
          Firmata.write(14);
        }
        if (IS_PIN_I2C(pin)) {
          Firmata.write(I2C);
          Firmata.write(1);                                       
        }
        Firmata.write(127);
      }
      Firmata.write(END_SYSEX);
      break;
    case PIN_STATE_QUERY:
      if (argc > 0) {
        byte pin = argv[0];
        Firmata.write(START_SYSEX);
        Firmata.write(PIN_STATE_RESPONSE);
        Firmata.write(pin);
        if (pin < TOTAL_PINS) {
          Firmata.write((byte)pinConfig[pin]);
          Firmata.write((byte)pinState[pin] & 0x7F);
          if (pinState[pin] & 0xFF80) Firmata.write((byte)(pinState[pin] >> 7) & 0x7F);
          if (pinState[pin] & 0xC000) Firmata.write((byte)(pinState[pin] >> 14) & 0x7F);
        }
        Firmata.write(END_SYSEX);
      }
      break;
    case ANALOG_MAPPING_QUERY:
      Firmata.write(START_SYSEX);
      Firmata.write(ANALOG_MAPPING_RESPONSE);
      for (byte pin = 0; pin < TOTAL_PINS; pin++) {
        Firmata.write(IS_PIN_ANALOG(pin) ? PIN_TO_ANALOG(pin) : 127);
      }
      Firmata.write(END_SYSEX);
      break;
  case PING_READ:{
      byte pulseDurationArray[4] = {
        (argv[2] & 0x7F) | ((argv[3] & 0x7F) << 7)
       ,(argv[4] & 0x7F) | ((argv[5] & 0x7F) << 7)
       ,(argv[6] & 0x7F) | ((argv[7] & 0x7F) << 7)
       ,(argv[8] & 0x7F) | ((argv[9] & 0x7F) << 7)
      };
      unsigned long pulseDuration = ((unsigned long)pulseDurationArray[0] << 24)
              + ((unsigned long)pulseDurationArray[1] << 16)
              + ((unsigned long)pulseDurationArray[2] << 8)
              + ((unsigned long)pulseDurationArray[3]);
      if(argv[1] == HIGH){
        pinMode(argv[0],OUTPUT);
        digitalWrite(argv[0],LOW);
        delayMicroseconds(2);
        digitalWrite(argv[0],HIGH);
        delayMicroseconds(pulseDuration);
        digitalWrite(argv[0],LOW);
      } else {
        digitalWrite(argv[0],HIGH);
        delayMicroseconds(2);
        digitalWrite(argv[0],LOW);
        delayMicroseconds(pulseDuration);
        digitalWrite(argv[0],HIGH);
      }
      unsigned long duration;
      byte responseArray[5];
      byte timeoutArray[4] = {
          (argv[10] & 0x7F) | ((argv[11] & 0x7F) << 7)
         ,(argv[12] & 0x7F) | ((argv[13] & 0x7F) << 7)
         ,(argv[14] & 0x7F) | ((argv[15] & 0x7F) << 7)
         ,(argv[16] & 0x7F) | ((argv[17] & 0x7F) << 7)
      };
      unsigned long timeout = ((unsigned long)timeoutArray[0] << 24)
                + ((unsigned long)timeoutArray[1] << 16)
                + ((unsigned long)timeoutArray[2] << 8)
                + ((unsigned long)timeoutArray[3]);
      pinMode(argv[0],INPUT);
      duration = pulseIn(argv[0], argv[1],timeout);
      responseArray[0] = argv[0];
      responseArray[1] = (((unsigned long)duration >> 24) & 0xFF) ;
      responseArray[2] = (((unsigned long)duration >> 16) & 0xFF) ;
      responseArray[3] = (((unsigned long)duration >> 8) & 0xFF);
      responseArray[4] = (((unsigned long)duration & 0xFF));
      Firmata.sendSysex(PING_READ,5,responseArray);
      break;
    }
  }
}

void enableI2CPins()
{
  byte i;
                                                                      
                                      
  for (i = 0; i < TOTAL_PINS; i++) {
    if (IS_PIN_I2C(i)) {
                                                                     
      setPinModeCallback(i, I2C);
    }
  }

  isI2CEnabled = true;

                                                                         
  Wire.begin();
}

                                                                  
void disableI2CPins() {
  isI2CEnabled = false;
                                                 
  queryIndex = -1;
}

                                                                                
          
                                                                                

void systemResetCallback()
{
                              
                                                               
  if (isI2CEnabled) {
    disableI2CPins();
  }

  for (byte i = 0; i < TOTAL_PORTS; i++) {
    reportPINs[i] = false;                                  
    portConfigInputs[i] = 0;                    
    previousPINs[i] = 0;
  }

  for (byte i = 0; i < TOTAL_PINS; i++) {
                                                          
                                                
    if (IS_PIN_ANALOG(i)) {
                                                
      setPinModeCallback(i, ANALOG);
    } else {
                                                          
      setPinModeCallback(i, OUTPUT);
    }

    servoPinMap[i] = 255;
  }
                                                
  analogInputsToReport = 0;

  detachedServoCount = 0;
  servoCount = 0;

                                                                       
                                                                        
    
                                                                      
                                                                     
                                        
                                                          
   
    
}

void setup()
{
  Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);

  Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
  Firmata.attach(DIGITAL_MESSAGE, digitalWriteCallback);
  Firmata.attach(REPORT_ANALOG, reportAnalogCallback);
  Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
  Firmata.attach(SET_PIN_MODE, setPinModeCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.attach(SYSTEM_RESET, systemResetCallback);

  Firmata.begin(57600);
  systemResetCallback();                            
}

                                                                                
         
                                                                                
void loop()
{
  byte pin, analogPin;

                                                                                
                                         
  checkDigitalInputs();

                                                                               
                                 
  while (Firmata.available())
    Firmata.processInput();

                                                                            
                                                                      
                                   

  currentMillis = millis();
  if (currentMillis - previousMillis > samplingInterval) {
    previousMillis += samplingInterval;
                                                                               
    for (pin = 0; pin < TOTAL_PINS; pin++) {
      if (IS_PIN_ANALOG(pin) && pinConfig[pin] == ANALOG) {
        analogPin = PIN_TO_ANALOG(pin);
        if (analogInputsToReport & (1 << analogPin)) {
          Firmata.sendAnalog(analogPin, analogRead(analogPin));
        }
      }
    }
                                                                       
    if (queryIndex > -1) {
      for (byte i = 0; i < queryIndex + 1; i++) {
        readAndReportData(query[i].addr, query[i].reg, query[i].bytes);
      }
    }
  }
}

