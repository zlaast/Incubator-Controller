#include "pid.h"

PID::PID()
{
    pwm_pin_ = kPwmPin;
    setpoint_ = 37.5f;
    Kp_ = 1.0f;
    Ki_ = 0.0f;
    Kd_ = 0.0f;
}


bool PID::Update(float process_value)
{
    float error = CalculateError(process_value);
    float output_raw = P(error); // I(error);
    uint8_t output = ClampOutput(output_raw);
    OCR1A = output;

    return true;
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


float PID::P(float error)
{
    return Kp_ * error;
}


float PID::I(float error)
{
    static float I_out = 0.0f;
    
    I_out += Ki_ * error;
    I_out = ClampOutput(I_out);

    return I_out;
}


float PID::D(float error)
{
    // For future implementation if desired
}

float PID::ClampOutput(float output)
{
    if (output > kPwmMaxOutput) return kPwmMaxOutput;
    else if (output < kPwmMinOutput) return kPwmMinOutput;
    else return output;
}


float PID::CalculateError(float process_value)
{
    return setpoint_ - process_value;
}