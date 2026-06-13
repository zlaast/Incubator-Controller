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

*********************************************************************/

#define F_CPU 8000000UL
#define LOW false
#define HIGH true

// LIBRARIES: user made
#include "src/pinout.h"
#include "src/port_operations.h"
#include "src/PID/pid.h"
#include "src/DHT22/dht22.h"
#include "src/Display/display.h"

// LIBRARIES: Standard
#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>

// STATE CONTROL
uint8_t error_code = 0;
enum States { kNormal, kAdjustSetpoint, kError };
States state = kNormal;

// CLASS OBJECTS
PID pid_controller;
DHT22Sensor sensor;
Display display(kDisplayClkPin, kDisplayDioPin);

// OPERATION TIMING
uint32_t start_time = 0;
uint32_t current_time = 0;
uint16_t elapsed_time = 0;
const uint8_t kSensorSamples = 3;
const uint16_t kSampleInterval = 2100;

// UI
bool button_pressed = false;
bool last_button_state = LOW;
const float kMinTemp = 15.0f;
const float kMaxTemp = 45.0f;

// OTHER
const float Kp = 14.0f;
const float Ki = 0.14f;
const float Kd = 0.0f;
const float setpoint = 37.5f;
const uint8_t kPwmMaxOutput = 159;


void setup()
{
    // Set clock speed to 8MHz
    clock_prescale_set(clock_div_1);
    
    // Setup PWM for 100kHz, ATtiny85
    SET_OUTPUT(kPwmPin);
    _delay_us(110);
    PLLCSR |= (1 << PLLE);
    while (!(PLLCSR & (1 << PLOCK)));
    PLLCSR |= (1 << PCKE);
    OCR1C = kPwmMaxOutput;
    TCCR1 = (1 << PWM1A) | (1 << COM1A1) | (1 << CS11) | (1 << CS10);

    // INITIALIZE CLASSES
    pid_controller.Begin(kPwmPin);
    sensor.Begin(kSensorPin);
    display.Begin();

    // Setup PID controller parameters
    pid_controller.UpdateTuning(Kp, Ki, Kd);
    pid_controller.UpdateSetpoint(setpoint);

    // Setup UI Controls
    SET_INPUT(kKnobPin);
    SET_INPUT(kButtonPin);
}


void loop()
{
    // UI: Check if user wishes to adjust setpoint
    bool current_button_state = READ_PIN(kButtonPin);
    button_pressed = (last_button_state == HIGH && current_button_state == LOW);
    last_button_state = current_button_state;

    // State Machine
    switch (state)
    {
        case kError:
            StateError();
            break;

        case kAdjustSetpoint:
            if (button_pressed)
            {
                state = kNormal;
                break;
            }

            StateAdjustSetpoint();
            break;

        case kNormal:
            if (button_pressed)
            {
                state = kAdjustSetpoint;
                break;
            }

            StateNormal();
            break;
    }
}


void StateError()
{
    current_time = millis();
    elapsed_time = current_time - start_time;

    if (elapsed_time > kSampleInterval)
    {
        switch (error_code)
        {
            case 1:
                display.ShowError(1);
                break;

            case 2:
                display.ShowError(2);
                break;
        }

        state = kNormal;
        start_time = current_time;
    }
}


void StateAdjustSetpoint()
{
    current_time = millis();

    const uint16_t kTimeout = 30000;
    static uint32_t timeout_start = 0;

    const uint8_t kSamples = 250;
    static uint8_t sample_count = 0;
    static uint32_t avg_sample_value = 0;

    int knob_value = analogRead(kKnobPin);
    int knob_position = map(knob_value, 0, 1023, kMaxTemp*10, kMinTemp*10);

    avg_sample_value += knob_position;
    sample_count++;

    if (sample_count > kSamples)
    {
        avg_sample_value /= kSamples;
        float new_setpoint = avg_sample_value * 0.1f;
        new_setpoint = round(new_setpoint*2.0f) * 0.5f;     // Rounds to nearest 0.5C

        pid_controller.UpdateSetpoint(new_setpoint);
        display.ShowTemperature(pid_controller.GetSetpoint());

        sample_count = 0;
        avg_sample_value = 0;
    }

    /* 
        Timeout: 
            If the user forgets they were adjusting the setpoint,
            then after ~30 seconds this mode will be automatically exited
            and will resume normal operation. 
            
            The setpoint will be set to whatever the knob is set to.
    */

    if ((current_time - timeout_start) > kTimeout)
    {
        timeout_start = current_time;
        state = kNormal;
    }
}


void StateNormal()
{
    /*
        During operation, the sensor is read every ~2.1 seconds with three readings (samples) being taken.
        These readings are averaged and then sent to the PID controller to be evaluated.
    */

    current_time = millis();
    elapsed_time = current_time - start_time;

    // The normal operations
    static float avg_temp = 0.0f;
    static uint8_t sample_count = 0;
    static bool display_temp = true;

    if (elapsed_time > kSampleInterval)
    {
        start_time = current_time;
        error_code = sensor.ReadSensor();

        // An error occurred! Stop everything!
        if (error_code)
        {
            pid_controller.FreezeOperations();
            state = kError;
            avg_temp = 0.0f;
            sample_count = 0;

            return;
        }

        sample_count++;
        avg_temp += sensor.GetTemperature();

        if (sample_count >= kSensorSamples)
        {
            pid_controller.Output(avg_temp/kSensorSamples);

            if (display_temp)
                display.ShowTemperature(sensor.GetTemperature());
            else
                display.ShowHumidity(sensor.GetHumidity());

            sample_count = 0;
            avg_temp = 0.0f;
            display_temp = !display_temp;
        }
    }
}