#include "pid.h"

uint16_t CHANGE_ME_COUNTER = 1;

PID::PID()
{
    pwm_pin_ = kPwmPin;
    setpoint_ = 37.5f;
    Kp_ = 1.0f;
    Ki_ = 0.0f;
    Kd_ = 0.0f;
    OCR1A = 0;
}


bool PID::Update(float process_value)
{
    float error = CalculateError(process_value);
    float output_raw = P(error) + I(error);
    uint8_t output = ClampOutput(output_raw, kPwmMinOutput, kPwmMaxOutput);
    OCR1A = output;

    Serial.print("Error: ");
    Serial.print(error);
    Serial.println();

    Serial.print("Output: ");
    Serial.print(OCR1A);
    Serial.println();
    Serial.print("Duty: ");
    Serial.print(OCR1A/159);
    Serial.print("%");
    Serial.println();
    Serial.println();

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
    Serial.print(CHANGE_ME_COUNTER);
    Serial.print(":");
    Serial.println();

    Serial.print("P: ");
    Serial.print(Kp_ * error);
    Serial.println();

    CHANGE_ME_COUNTER++;

    return Kp_ * error;
}


float PID::I(float error)
{
    static float I = 0.0f;
    
    I += Ki_ * error;
    I = ClampOutput(I, kPwmMinOutput, kPwmMaxOutput);

    Serial.print("I: ");
    Serial.print(I);
    Serial.println();

    return I;
}


float PID::D(float error)
{
    // For future implementation if desired
}


float PID::CalculateError(float process_value)
{
    return setpoint_ - process_value;
}


float PID::ClampOutput(float input, uint8_t min, uint8_t max)
{
    if (input < min)
        return min;
    else if (input > max)
        return max;

    return input;
}