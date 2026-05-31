/*********************************************************************

    File: pid.h

    Author: Z'Laast Allicock
    
    Description:
        A PID controller for temperature control.

    Notes:
       - Despite this being called a PID controller, this project
        only needs a PI controller. I wrote this to be extendable
        so that the D term can be included easily.

        - Before outputting a PWM signal, we must first normalize the
        output for the PID calculation (done in the Update function).
        
        The normalization formula is: value = (x - x_min)/(x_max - x_min)

        The output for the PID calculation is, though, based on the *error*,
        which in turn is based on the setpoint, which can change. Thus x_max
        and x_min are the max and min error that may occur! Since the setpoint
        can change, we must calculate the max and min error that may occur
        *before* we can normalize. This is done by the CalculateNormalizationBounds
        function.

        - Based on the series of posts by Brett Beuaregard.
        http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

*********************************************************************/

#pragma once

#include <Arduino.h>
#include "../pinout.h"

class PID
{
    public:
        PID();
        bool Update(float process_variable);
        void UpdateSetpoint(float new_setpoint);
        void UpdateTuning(float Kp, float Ki, float Kd);

    private:
        uint8_t pwm_pin_;
        float setpoint_;
        float Kp_;
        float Ki_;
        float Kd_;
        float x_max_;   // Min value for normalization
        float x_min_;   // Max value for normalization

        float P(float error);
        float I(float error);
        float D(float error);
        float Normalize(float raw_output);
        float ClampOutput(float output);
        float CalculateError(float process_variable);
        void CalculateNormalizationBounds();
};