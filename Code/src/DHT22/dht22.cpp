#include "dht22.h"

#define DELAY_AMOUNT 4
#define CUTOFF 8

void DHT22::init_(uint8_t data_pin)
{
    this->data_pin = data_pin;
    _delay_ms(1500);
}

void DHT22::send_request_()
{
    SET_OUTPUT(data_pin);

    WRITE_LOW(data_pin);
    _delay_ms(5);
    
    WRITE_HIGH(data_pin);
    _delay_us(20);
}

bool DHT22::receive_request_()
{
    SET_INPUT(data_pin);

    bool pin_state = LOW;
    bool done = false;
    long timeout_start = millis();

    for (uint8_t bit = 0; bit < 42;)
    {
        pin_state = READ_PIN(data_pin);
        _delay_us(DELAY_AMOUNT);

        if (pin_state == HIGH)
        {
            samples[bit]++;
            done = false;
        }
        else if (pin_state == LOW and !done)
        {
            bit++;
            done = true;
        }

        if (millis() - timeout_start > 6)
            return false;
    }

    return true;
}

float DHT22::decode_request_(Data data)
{
    uint8_t start = 2;
    uint8_t end = 18;

    switch (data)
    {
        case HUMIDITY:
            start = 2;
            end = 18;
        break;

        case TEMPERATURE:
            start = 18;
            end = 34;
        break;
    }

    // Decode samples into a binary string
    char data_string[16] = "";
    char *endptr;
    
    for (uint8_t bit = start; bit < end; bit++)
    {
        if (samples[bit] < CUTOFF)
            strcat(data_string, "0");
        else
            strcat(data_string, "1");
    }

    // Check if temperature is negative
    if (data == TEMPERATURE && data_string[0] == '1')
    {
        data_string[0] = '0';
        return -strtol(data_string, &endptr, 2)/10.0f;
    }

    return strtol(data_string, &endptr, 2)/10.0f;
}

bool DHT22::read_sensor_()
{
    send_request_();

    if (!receive_request_())
        return false;

    humidity = decode_request_(HUMIDITY);
    temperature = decode_request_(TEMPERATURE);
    
    memset(samples, 0, sizeof(samples));

    return true;
}