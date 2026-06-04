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
#include "../constants.h"
#include "../port_operations.h"

class DHT22
{
    private:
        uint8_t data_pin_;                          // Pin that connects to the DHT22 DAT pin
        uint8_t buffer_[42];
        float humidity_;                            // Percent relative humidity (% RH)
        float temperature_;                         // Celsius

        void RequestData();
        bool ReceiveData();
        bool DecodeData();

        // Helper Functions
        void BubbleSort(uint8_t array[]);
        uint8_t Calculate_kCutoff();
        float Cost(uint8_t start, uint8_t end);

    public:
        DHT22(uint8_t DAT_pin);
        uint8_t ReadSensor();
        float GetHumidity()    const { return humidity_; }
        float GetTemperature() const { return temperature_; }
};