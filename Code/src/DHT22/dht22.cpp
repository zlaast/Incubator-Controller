#include "dht22.h"

#define LOW false
#define HIGH true

const uint8_t kCutoff = 8;
const uint8_t kDelayAmount = 4;     // Microseconds
const uint8_t kTimeout = 6;         // Seconds


DHT22::DHT22(uint8_t DAT_pin)
{
    data_pin_ = DAT_pin;
    _delay_ms(1100);                // Wait until sensor is stable
}


uint8_t DHT22::ReadSensor()
{
    SendRequest();

    // If sensor doesn't respond, return error code 1
    if (!ReceiveRequest())
        return 1;

    // If checksum has failed, return error code 2
    if (!DecodeRequest())
        return 2;

    memset(samples_, 0, sizeof(samples_));

    return 0;
}


void DHT22::SendRequest()
{
    SET_OUTPUT(data_pin_);

    WRITE_LOW(data_pin_);
    _delay_ms(5);
    
    WRITE_HIGH(data_pin_);
    _delay_us(20);
}


bool DHT22::ReceiveRequest()
{
    SET_INPUT(data_pin_);

    bool pin_state = LOW;
    bool done = false;
    uint32_t timeout_start = millis();

    for (uint8_t bit = 0; bit < 42;)
    {
        pin_state = READ_PIN(data_pin_);
        _delay_us(kDelayAmount);        // Pin state doesn't seem to read correctly without a small delay
        
        if (pin_state == HIGH)
        {
            samples_[bit]++;
            done = false;
        }

        else if (pin_state == LOW && !done)
        {
            bit++;
            done = true;
        }

        // Sensor hasn't responded within kTimeout seconds and has timed out
        // Return that sensor has not responded
        if (millis() - timeout_start > kTimeout)
            return false;
    }

    return true;
}

/*
    DecodeRequest Explanation.

    kCutoff is dependent on clock speed and kDelayAmount.

    In the function ReceiveRequest, the sensor pin is polled
    on whether it's high or low.

    When then pin goes high, a counter starts and samples how many
    times the pin is read high. The speed of which the pin is sampled
    is determined by the clock speed and kDelayAmount.

    The sensor sends a ~27us pulse to represent a 0.
    It sends a ~70us pulse to represent a 1.
    
    Experimentally (by printing the samples_ array to the console),
    it was found that 0 bits are sampled ~5 times and 1 bits are sampled ~12 times
    at the given clock speed (8MHz) and kDelayAmount (4us).

    Thus a cutoff of 8 was chosen. If below 8 samples then the bit is a 0, if above
    8 samples the bit is a 1.

    We can convert these into a bitstream while automatically converting to a
    uint16_t by the following:

        (value << 1) causes a 0 to be bit shifted into value.
        This is then bitwise ORed with (samples_[bit] > 8), which
        is a boolean statement and therefore will output either a 0 or 1.

    Also, if the data is for temperature, then the first bit is a sign bit.
    If the first bit is a 1, then the temperature is negative.
    After that, we must remove the sign bit by bitwise ANDing with 0x7FFF

    Note that start = 2. This is because "bits" 0 and 1 are junk data.
*/

bool DHT22::DecodeRequest()
{
    uint8_t bytes[5] = {0};
    for (uint8_t byte = 0; byte < 5; byte++)
    {
        uint8_t start = 2 + (byte * 8);
        uint8_t end = start + 8;

        for (uint8_t bit = start; bit < end; bit++)
        {
            bytes[byte] = (bytes[byte] << 1) | (samples_[bit] > kCutoff);
        }
    }

    uint8_t checksum = bytes[0] + bytes[1] + bytes[2] + bytes[3];

    if (checksum != bytes[4])
        return false;

    uint16_t humidity = (bytes[0] << 8) | bytes[1];
    humidity_ = humidity * 0.1f;

    uint16_t temperature = (bytes[2] << 8) | bytes[3];
    bool negative_temp = temperature & 0x8000;
    temperature &= 0x7FFF;
    temperature_ = negative_temp ? -(temperature * 0.1f) : (temperature * 0.1f);

    return true;
}