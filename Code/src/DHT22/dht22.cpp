/*
    Error Codes:
        0 - No error.
        1 - No sensor data. Sensor may be disconnected.
        2 - Sensor data is incorrect. Failed checksum.
*/

#include "dht22.h"

#define LOW false
#define HIGH true


DHT22Sensor::DHT22Sensor(uint8_t data_pin)
{
    data_pin_ = data_pin;
    binarize_threshold = 0;
    humidity_ = 0.0f;
    temperature_ = 0.0f;
    ClearDataBuffer();

    _delay_ms(1100);    // Sensor needs ~1s to stabilize from power-on
}


uint8_t DHT22Sensor::ReadSensor()
{
    uint8_t error_code = 0;

    SendRequest();

    if (!ReceiveResponse())
    {
        ClearDataBuffer();
        error_code = 1;
        
        return error_code;
    }

    if (!DecodeResponse())
    {
        // Decoding could fail due to a wrong binarize threshold, leading to garbled data.
        // If the clock drifts the sample amounts in the buffer may change, leading to a threshold that is no longer applicable.
        // While this isn't likely to happen, recalculating the threshold just may fix the problem automatically.
        FindBinarizeThreshold();

        ClearDataBuffer();
        error_code = 2;
        
        return error_code;
    }

    ClearDataBuffer();

    return error_code;
}


void DHT22Sensor::ClearDataBuffer()
{
    memset(data_buffer_, 0, sizeof(data_buffer_));
}


void DHT22Sensor::SendRequest()
{
    SET_OUTPUT(data_pin_);

    WRITE_LOW(data_pin_);
    _delay_ms(5);

    WRITE_HIGH(data_pin_);
    _delay_us(25);
}


bool DHT22Sensor::ReceiveResponse()
{
    const uint8_t kTimeoutMilliseconds = 10;
    uint32_t response_start_time = millis();

    SET_INPUT(data_pin_);
    bool pin_state = LOW;
    bool done_sampling = false;

    for (uint8_t bit = 0; bit < sizeof(data_buffer_);)
    {
        pin_state = READ_PIN(data_pin_);

        if (pin_state == HIGH)
        {
            data_buffer_[bit]++;
            done_sampling = false;
        }
        else if (pin_state == LOW && !done_sampling)
        {
            bit++;
            done_sampling = true;
        }

        if ((millis() - response_start_time) > kTimeoutMilliseconds)
            return false;
    }

    return true;
}


bool DHT22Sensor::DecodeResponse()
{
    uint8_t bytes[5] = {0};

    if (binarize_threshold == 0)
        FindBinarizeThreshold();

    BinarizeBufferData(bytes);

    if (isBinarizedDataValid(bytes) == false)
        return false;

    ExtractHumidityAndTemperature(bytes);

    return true;
}


void DHT22Sensor::BinarizeBufferData(uint8_t buffer[])
{
    for (uint8_t byte = 0; byte < 5; byte++)
    {
        // First two bits are junk data and are skipped
        uint8_t byte_start = 2 + (byte * 8);
        uint8_t byte_end = byte_start + 8;

        for (uint8_t bit = byte_start; bit < byte_end; bit++)
        {
            // 1. Shift a 0 into the current byte (the 1 is for how many 0-bits we're shifting in, which is just one.)
            // 2. Determine if 1 or 0 in data buffer. Returns bool which is a 1 or 0.
            // 3. Bitwse-OR with shifted in 0. Ta-da! Binarized!
            buffer[byte] <<= 1;
            bool isOne = data_buffer_[bit] > binarize_threshold;
            buffer[byte] |= isOne;
        }
    }
}


bool DHT22Sensor::isBinarizedDataValid(uint8_t buffer[])
{
    // Checksum. buffer[4] is the checksum returned by the sensor
    uint8_t checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
    
    if (checksum != buffer[4])
        return false;

    return true;
}


void DHT22Sensor::ExtractHumidityAndTemperature(uint8_t buffer[])
{
    uint16_t humidity = (buffer[0] << 8) | buffer[1];
    humidity_ = humidity * 0.1f;

    uint16_t temperature = (buffer[2] << 8) | buffer[3];
    bool negative_temp = temperature & 0x8000;
    temperature &= 0x7FFF;
    temperature_ = negative_temp ? -(temperature * 0.1f) : (temperature * 0.1f);
}


// Find Binarize Threshold using ckmeans algorithm
uint8_t sums[43] = {0};
uint8_t sums_of_squares[43] = {0};

void DHT22Sensor::FindBinarizeThreshold()
{
    // Create copy of data_buffer_ and sort it (ascending)
    uint8_t temp_buffer[42] = {0};
    memcpy(temp_buffer, data_buffer_, sizeof(data_buffer_));
    BubbleSort(temp_buffer);

    // Find threshold by using ckmeans (optimal k-means) using 1d data and k=2
    uint8_t best_index = 0;
    uint8_t kBufferSize = sizeof(temp_buffer);
    float best_cost = 99999.0f;

    for (int i = 0; i < kBufferSize; i++)
    {
        sums[i+1] = sums + temp_buffer[i];
        sums_of_squares[i+1] = sums_of_squares[i] + temp_buffer[i] * temp_buffer[i];
    }

    for (uint8_t split = 1; split < (kBufferSize - 1); split++)
    {
        float cost = Ckmeans_CalculateCost(0, split) + Ckmeans_CalculateCost(split, kBufferSize);

        if (cost < best_cost)
        {
            best_cost = cost;
            best_index = split;
        }
    }

    binarize_threshold = (temp_buffer[best_index - 1] + temp_buffer[best_index]) * 0.5f;
}


void DHT22Sensor::BubbleSort(uint8_t array[])
{
    const uint8_t kArraySize = sizeof(array);

    for (uint8_t i = 0; i < (kArraySize-1); i++)
    {
        bool swapped = false;

        for (uint8_t j = 0; j < (kArraySize-i-1); ++j)
        {
            if (array[j] > array[j+1])
            {
                uint8_t temp = array[j];
                array[j] = array[j+1];
                array[j+1] = temp;

                swapped = true;
            }
        }

        if (!swapped)
            break;
    }
}


float DHT22Sensor::Ckmeans_CalculateCost(uint8_t start, uint8_t end)
{
    float sum = sums[end] - sums[start];
    float sum_of_squares = sums_of_squares[end] - sums_of_squares[start];
    uint8_t n = end - start;

    if (n <= 1)
        return 0.0f;

    return sum_of_squares - (sum * sum)/n;
}