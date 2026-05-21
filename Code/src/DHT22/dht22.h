////////////////////////////////////////////////////////////////////////
// NAME:               dht22.h
//
// PURPOSE:            Library for reading a DHT22 sensor
//
// AUTHOR:             Z'Laast Allicock
////////////////////////////////////////////////////////////////////////

#ifndef DHT22_H
#define DHT22_H

#include <Arduino.h>
#include <string.h>
#include "../pin_operations.h"

class DHT22
{
    public:
        void init_(uint8_t data_pin);
        bool read_sensor_();
        float get_humidity_()    const { return humidity; }
        float get_temperature_() const { return temperature; }

    private:
        uint8_t data_pin;                       // Pin that connects to the DHT22 DAT pin
        uint8_t samples[42];
        float humidity;                         // Percent relative humidity (% RH)
        float temperature;                      // Celsius
        enum Data { HUMIDITY, TEMPERATURE };

        void send_request_();
        bool receive_request_();
        float decode_request_(Data data);
};

#endif