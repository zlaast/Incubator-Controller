#pragma once

// CONTROLS PINS [MCU INPUTS]
// const uint8_t kKnobPin          = PB2;
const uint8_t kKnobPin = 14;    // CHANGE ME
const uint8_t kButtonPin        = PB5;

// TEMP AND HUMIDITY SENSOR PIN [MCU INPUTS]
const uint8_t kSensorPin        = PB4;

// DISPLAY PINS [MCU OUTPUTS]
const uint8_t kDisplayClkPin    = PB0;
const uint8_t kDisplayDioPin    = PB3;

// TEC HEATER PWN PIN [MCU OUTPUT]
// MUST BE PB1 FOR ATTINY85
const uint8_t kPwmPin           = PB1;