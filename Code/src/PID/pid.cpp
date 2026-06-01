#include "pid.h"

PID::PID()
{
    pwm_pin_ = kPwmPin;
    setpoint_ = 37.5f;
    Kp_ = 1.0f;
    Ki_ = 0.0f;
    Kd_ = 0.0f;

    CalculateMaxError();
}


bool PID::Update(float process_variable)
{
    float error = CalculateError(process_variable);
    float output_raw = P(error); // I(error);
    float output_normalized = Normalize(output_raw);

    if (output_normalized < 0.0f || output_normalized > 1.0f)
    {
        OCR1A = 0;
        return false;
    }

    uint8_t duty_cycle = output_normalized * kPwmMaxOutput;
    uint8_t output = ClampOutput(duty_cycle);
    OCR1A = output;

    return true;
}


void PID::UpdateSetpoint(float new_setpoint)
{
    setpoint_ = new_setpoint;
    CalculateMaxError();
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


float PID::Normalize(float raw_output)
{
    return (raw_output - x_min_)/(x_max_ - x_min_);
}


float PID::ClampOutput(float output)
{
    if (output > kPwmMaxOutput)
        return kPwmMaxOutput;
    else if (output < kPwmMinOutput)
        return kPwmMinOutput;
    else
        return output;
}


float PID::CalculateError(float process_variable)
{
    return setpoint_ - process_variable;
}


void PID::CalculateMaxError()
{
    x_min_ = (Kp_ + Ki_ + Kd_)*(setpoint_ - kMaxTemp);
    x_max_ = (Kp_ + Ki_ + Kd_)*(setpoint_ - kMinTemp);
}