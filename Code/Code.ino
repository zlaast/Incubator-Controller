/*********************************************************************

    File: Code.ino

    Author: Z'Laast Allicock
    
    Description:
        Main file that controls the incubator temperature
        via a PI controller.

    Notes:
        - This code was written for an ATtiny85 microcontroller. 
        The PWM signal must be on PB1 for this microcontroller.
        The code can be modified for another controller though.

        - The clock speed is set to fastest because the DHT22 sensor
        library needs to write and read in the low microseconds
        range. For an Attiny85, this is 8MHz. This was performed
        by the function clock_prescale_set()

    Error Codes:
        0 - No error. This should never be displayed during normal operation.
        1 - No sensor data. Sensor may be disconnected.
        2 - Sensor data is incorrect. Failed checksum.
        3 - Temperature out of expected bounds (too hot/too cold)

*********************************************************************/

// Includes
#include "src/pinout.h"
#include "src/constants.h"
#include "src/port_operations.h"
#include "src/PID/pid.h"
#include "src/DHT22/dht22.h"
#include "src/Display/display.h"

// Libraries
#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>

// Macros
#define F_CPU 16000000UL    // CHANGE ME

// Constants
const uint8_t kSensorSamples = 3;
const uint16_t kSampleInterval = 2100;

// Enums
enum States { kNormal, kAdjustSetpoint, kError };

// Class Instances
PID pid_controller;
Display display(7, 6);      // CHANGE ME
DHT22 sensor(kSensorPin);

// Global Variables
uint8_t error_code = 0;
uint16_t elapsed_time = 0;
uint32_t start_time = 0;
uint32_t current_time = 0;
States state = kNormal;

// Button Variables
bool last_button_state = LOW;
bool button_pressed = false;


void setup()
{
    clock_prescale_set(clock_div_1);

    // Setup PID parameters
    pid_controller.UpdateTuning(1.5f, 0.0f, 0.0f);
    pid_controller.UpdateSetpoint(37.5f);

    // Setup PWM (for Arduino Nano) CHANGE ME
    // This is setup for a 100kHz signal
    SET_OUTPUT(9);
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
    ICR1 = kPwmMaxOutput;
    Serial.begin(9600);

    // // Setup PWM for PID. 100kHz frequency. ATtiny85 specific
    // SET_OUTPUT(kPwmPin); // MUST be PB1 for ATtiny85
    // PLLCSR |= (1 << PLLE);
    // _delay_us(110);
    // while (!(PLLCSR & (1 << PLOCK)));
    // PLLCSR |= (1 << PCKE);
    // OCR1C = kPwmMaxOutput;
    // TCCR1 = (1 << PWM1A) | (1 << COM1A1) | (1 << CS11) | (1 << CS10);

    // Setup Button and Knob
    SET_INPUT(kKnobPin);
    SET_INPUT(kButtonPin);
}


void loop()
{
    // Poll if button is pressed
    bool current_button_state = READ_PIN(kButtonPin);
    button_pressed = (last_button_state == HIGH && current_button_state == LOW);
    last_button_state = current_button_state;

    // State Machine
    switch (state)
    {
        case kNormal:
            if (button_pressed)
            {
                state = kAdjustSetpoint;
                break;
            }

            NormalState();
            break;
        
        case kAdjustSetpoint:
            if (button_pressed)
            {
                state = kNormal;
                display.Clear();
                break;
            }

            AdjustSetpointState();
            break;

        case kError:
            ErrorState();
            break;
    }
}


void NormalState()
{
    // Update Interval
    current_time = millis();
    elapsed_time = current_time - start_time;

    // PID Variables
    static uint8_t sample_count = 0;
    static float avg_temp = 0.0f;

    // Display Variables
    static bool display_temp = true;

    // Logic
    if (elapsed_time > kSampleInterval)
    {
        start_time = current_time;
        error_code = sensor.ReadSensor();
        
        if (error_code)
        {
            state = kError;
            return;
        }

        sample_count++;
        avg_temp += sensor.GetTemperature();

        if (sample_count >= kSensorSamples)
        {
            if (pid_controller.Update(avg_temp/kSensorSamples))
            {
                if (display_temp)
                    display.ShowTemperature(sensor.GetTemperature());
                else
                    display.ShowHumidity(sensor.GetHumidity());
            }
            else
            {
                state = kError;
                error_code = 3;
            }

            sample_count = 0;
            avg_temp = 0.0f;
            display_temp = !display_temp;
        }
    }
}


void AdjustSetpointState()
{
    const uint8_t kMaxSamples = 250;
    static uint8_t sample_count = 0;
    static uint32_t avg_value = 0;

    int value = analogRead(kKnobPin);
    int position = map(value, 0, 1023, kMaxTemp*10, kMinTemp*10);

    avg_value += position;
    sample_count++;

    if (sample_count > kMaxSamples)
    {
        avg_value /= kMaxSamples;
        float new_setpoint = avg_value / 10.0f;

        pid_controller.UpdateSetpoint(new_setpoint);
        display.ShowTemperature(pid_controller.GetSetpoint());

        sample_count = 0;
        avg_value = 0;
    }
}


void ErrorState()
{
    current_time = millis();
    elapsed_time = current_time - start_time;

    if (elapsed_time > kSampleInterval)
    {
        switch(error_code)
        {            
            case 1:
                display.ShowError(1);
                break;

            case 2:
                display.ShowError(2);
                break;

            case 3:
                display.ShowError(3);
                break;
        }

        state = kNormal;
        start_time = current_time;
    }
}