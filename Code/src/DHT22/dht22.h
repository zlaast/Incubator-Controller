/*********************************************************************

    File: dht22.h

    Author: Z'Laast Allicock
    
    Description:
        Library to read a DHT22 sensor.

    Notes:
        Clock needs to be at least 8MHz

*********************************************************************/

#pragma once

#include <Arduino.h>
#include <string.h>
#include "../port_operations.h"

class DHT22
{
    private:
        enum Climate { kHumidity, kTemperature };
        uint8_t data_pin_;                          // Pin that connects to the DHT22 DAT pin
        uint8_t samples_[42];
        float humidity_;                            // Percent relative humidity (% RH)
        float temperature_;                         // Celsius

        void SendRequest();
        bool ReceiveRequest();
        bool DecodeRequest();

    public:
        DHT22(uint8_t DAT_pin);
        uint8_t ReadSensor();
        float GetHumidity()    const { return humidity_; }
        float GetTemperature() const { return temperature_; }
};