//////////////////////////////////////////////////////////////////////////////////////////
// NAME:               Code.ino
//
// PURPOSE:            Controls incubator temperature
//
// AUTHOR:             Z'Laast Allicock
//////////////////////////////////////////////////////////////////////////////////////////

#define F_CPU 8000000UL

#include <avr/power.h>
#include <util/delay.h>

#include "src/constants.h"
#include "src/pin_operations.h"
#include "src/DHT22/dht22.h"

DHT22 sensor;

void setup()
{
    // Set clock speed to 8MHz (by disabling prescaling).
    // The DHT Library needs to be able to write signals quickly (microseconds)
    // so we need the clock to be fast enough for that.
    clock_prescale_set(clock_div_1);

    // Setup sensor
    // sensor.init_(sensorPin);
    sensor.init_(PB0);

    // DELETE ME
    Serial.begin(9600);
}

void loop()
{
    if(sensor.read_sensor_())
    {
        // Do something with temperature and humidity

        _delay_ms(2100);
    }
    else
    {
        // Throw error here
        continue;
    }
    
}