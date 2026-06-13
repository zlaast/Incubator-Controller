#include "pid.h"

// PWM values to produce 100kHz signal on ATtiny85
// See setup() in Code.ino which sets up the 100kHz PWM output

const float kPwmMinOutput = 0;
const float kPwmMaxOutput = 159;

void PID::Begin(uint8_t pwm_pin)
{
    pwm_pin_ = pwm_pin;
    setpoint_ = 37.5f;
    Kp_ = 1.0f;
    Ki_ = 0.0f;
    Kd_ = 0.0f;
    OCR1A = 0;
}


void PID::Output(float process_value)
{
    float error = CalculateError(process_value);
    float output_raw = P(error) + I(error) + D(error);
    uint8_t output = ClampOutput(output_raw, kPwmMinOutput, kPwmMaxOutput);
    OCR1A = output;
}


void PID::UpdateSetpoint(float new_setpoint)
{
    setpoint_ = new_setpoint;
}


void PID::UpdateTuning(float Kp, float Ki, float Kd)
{
    Kp_ = Kp;
    Ki_ = Ki;
    Kd_ = Kd;
}


void PID::FreezeOperations()
{
    OCR1A = 0;
}


float PID::P(float error)
{
    return Kp_ * error;
}


float PID::I(float error)
{
    static float I = 0.0f;
    
    I += Ki_ * error;
    I = ClampOutput(I, kPwmMinOutput, kPwmMaxOutput);

    return I;
}


float PID::D(float error)
{
    // For future implementation if desired
    return 0.0f;
}


float PID::CalculateError(float process_value)
{
    return setpoint_ - process_value;
}


float PID::ClampOutput(float input, float min_val, float max_val)
{
    if (input < min_val)
        return min_val;
    else if (input > max_val)
        return max_val;

    return input;
}