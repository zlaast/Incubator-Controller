/*********************************************************************

    File: dht22.h

    Author: Z'Laast Allicock
    
    Description:
        Library to read a DHT22 temperature/humidity sensor.

    Notes:
        Clock needs to be at least 8MHz

*********************************************************************/

#pragma once

#include <Arduino.h>
#include <util/delay.h>
#include "../port_operations.h"

class DHT22Sensor
{
    public:
        DHT22Sensor(uint8_t data_pin);
        uint8_t ReadSensor();
        float GetHumidity() const { return humidity_; }
        float GetTemperature() const { return temperature_; }

    private:
        uint8_t data_pin_;
        uint8_t data_buffer_[42];
        uint8_t binarize_threshold;
        float humidity_;            // Relative Humidity
        float temperature_;         // Celsius

        // Primary Functions
        void ClearDataBuffer();
        void SendRequest();
        bool ReceiveResponse();
        bool DecodeResponse();
        
        // DecodeResponse Helper Functions
        void BinarizeBufferData(uint8_t buffer[]);
        bool isBinarizedDataValid(uint8_t buffer[]);
        void ExtractHumidityAndTemperature(uint8_t buffer[]);
        
        // Find Binarize Threshold using ckmeans algorithm
        void FindBinarizeThreshold();
        void BubbleSort(uint8_t array[]);
        float Ckmeans_CalculateCost(uint8_t start, uint8_t end);
};