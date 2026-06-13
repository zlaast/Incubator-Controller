/*********************************************************************

    File: pid.h

    Author: Z'Laast Allicock
    
    Description:
        A PID controller for temperature control.

    Notes:
       - Despite this being called a PID controller, this project
        only needs a PI controller. I wrote this to be extendable
        so that the D term can be included easily.

        - Based on the series of posts by Brett Beauregard.
        http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

*********************************************************************/

#pragma once

#include <Arduino.h>
#include "../pinout.h"

class PID
{
    public:
        void Begin(uint8_t pwm_pin);
        void Output(float process_value);
        void UpdateSetpoint(float new_setpoint);
        void UpdateTuning(float Kp, float Ki, float Kd);
        void FreezeOperations();    // If an error occurs, use this to stop controller until error is fixed
        float GetSetpoint() const { return setpoint_; }

    private:
        uint8_t pwm_pin_;
        float setpoint_;
        float Kp_;
        float Ki_;
        float Kd_;

        float P(float error);
        float I(float error);
        float D(float error);
        float CalculateError(float process_value);
        float ClampOutput(float input, float min_val, float max_val);
};