#ifndef CONSTANTS_H
#define CONSTANTS_H

// CONTROLS PINS [MCU INPUTS]
const uint8_t buttonPin     = PB1;
const uint8_t knobPin       = PB2;

// TEMP AND HUMIDITY SENSOR PIN [MCU INPUTS]
const uint8_t sensorPin     = PB4;

// DISPLAY PINS [MCU OUTPUTS]
const uint8_t displayCLKPin = PB0;
const uint8_t displayDIOPin = PB3;

// TEC HEATER PWN PIN [MCU OUTPUT]
const uint8_t pwmPin        = PB5;

// TIMER CONSTANTS
const uint16_t sensorReadTime = 2100; // 2.1 seconds
const uint16_t displayUpdateTime = 6300; // 6.3 seconds

// PID CONTROLLER CONSTANTS
const uint8_t KP = 1;
const uint8_t KI = 1;

// OTHER CONSTANTS
const float defaultTemp = 37.5;

#endif