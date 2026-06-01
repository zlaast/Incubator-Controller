#pragma once

#define LOW false
#define HIGH true

// PWM Signal: Counter for ATtiny85
const float kPwmMinOutput = 0;
const float kPwmMaxOutput = 159;

// Max and min temperature the incubator should ever see
const float kMinTemp = 15.0f;
const float kMaxTemp = 45.0f;