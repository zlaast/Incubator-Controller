#include "dht22.h"

uint8_t kCutoff = 1;
bool found_cutoff = false;
const uint8_t kDelayAmount = 3;     // Microseconds
const uint8_t kTimeout = 10000;     // Milliseconds
const uint8_t kBufferSize = 42;

struct BufferRAII
{
    uint8_t* buffer;
    size_t size;

    ~BufferRAII()
    {
        memset(buffer, 0, size);
    }
};


DHT22::DHT22(uint8_t DAT_pin)
{
    data_pin_ = DAT_pin;
    _delay_ms(1100);                // Wait at least 1s for sensor to stabilize
}


uint8_t DHT22::ReadSensor()
{
    BufferRAII clean { buffer_, sizeof(buffer_) };

    RequestData();

    if (!ReceiveData())
        return 1;

    if (!DecodeData())
        return 2;

    return 0;
}


void DHT22::RequestData()
{
    SET_OUTPUT(data_pin_);

    WRITE_LOW(data_pin_);
    _delay_ms(5);
    
    WRITE_HIGH(data_pin_);
    _delay_us(20);
}


bool DHT22::ReceiveData()
{
    SET_INPUT(data_pin_);

    bool pin_state = LOW;
    bool done = false;
    uint32_t timeout_start = millis();

    for (uint8_t bit = 0; bit < kBufferSize;)
    {
        pin_state = READ_PIN(data_pin_);
        _delay_us(kDelayAmount);        // Pin state doesn't seem to read correctly without a small delay
        
        if (pin_state == HIGH)
        {
            buffer_[bit]++;
            done = false;
        }

        else if (pin_state == LOW && !done)
        {
            bit++;
            done = true;
        }

        // Sensor hasn't responded within kTimeout seconds and has timed out
        // Return that sensor has not responded
        if ((millis() - timeout_start) > kTimeout)
            return false;
    }

    return true;
}

/*
    DecodeData Explanation.

    kCutoff is dependent on clock speed and kDelayAmount.

    In the function ReceiveData, the sensor pin is polled
    on whether it's high or low.

    When then pin goes high, a counter starts and samples how many
    times the pin is read high. The speed of which the pin is sampled
    is determined by the clock speed and kDelayAmount.

    The sensor sends a ~27us pulse to represent a 0.
    It sends a ~70us pulse to represent a 1.
    
    Experimentally (by printing the buffer_ array to the console),
    it was found that 0 bits are sampled ~4 times and 1 bits are sampled ~10 times
    at the given clock speed (8MHz) and kDelayAmount (3us).

    Thus a cutoff of 7 was chosen. If below 7 samples then the bit is a 0, if above
    8 samples the bit is a 1.

    We can convert these into a bitstream while automatically converting to a
    uint16_t by the following:

        (value << 1) causes a 0 to be bit shifted into value.
        This is then bitwise ORed with (buffer_[bit] > 7), which
        is a boolean statement and therefore will output either a 0 or 1.

    Also, if the data is for temperature, then the first bit is a sign bit.
    If the first bit is a 1, then the temperature is negative.
    After that, we must remove the sign bit by bitwise ANDing with 0x7FFF

    Note that start = 2. This is because "bits" 0 and 1 are junk data.
*/

bool DHT22::DecodeData()
{
    if (!found_cutoff)
    {
        uint8_t temp_buffer[42] = {0};
        memcpy(temp_buffer, buffer_, sizeof(buffer_));
        BubbleSort(temp_buffer);
        kCutoff = Calculate_kCutoff();
        found_cutoff = true;
    }

    uint8_t bytes[5] = {0};
    for (uint8_t byte = 0; byte < 5; byte++)
    {
        uint8_t start = 2 + (byte * 8);
        uint8_t end = start + 8;

        for (uint8_t bit = start; bit < end; bit++)
        {
            bytes[byte] = (bytes[byte] << 1) | (buffer_[bit] > kCutoff);
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


void DHT22::BubbleSort(uint8_t array[])
{
    for (uint8_t i = 0; i < (kBufferSize-1); i++)
    {
        bool swapped = false;

        for (uint8_t ii = 0; ii < (kBufferSize-i-1); ++ii)
        {
            if (array[ii] > array[ii+1])
            {
                uint8_t temp = array[ii];
                array[ii] = array[ii+1];
                array[ii+1] = temp;

                swapped = true;
            }
        }

        if (!swapped)
            break;
    }
}


/*
    Calculates kCutoff


    In the DecodeData function, you need to decide whether a bit
    is a 0 or a 1 based on the sample count in buffer_. This is done
    by comparing the count to kCutoff. If the count is less than kCutoff
    then the bit send is a 0, otherwise a 1.

    But the amount of samples we count are dependent on kDelayAmount and
    the clock speed, which means kCutoff needs to shift accordingly too.

    What we *do* know is that we'll have a bunch of small sample counts,
    and a bunch of large sample counts. Take this example data:

    Unsorted
    0,11,3,3,3,4,4,3,9,3,4,3,8,3,9,9,3,10,3,3,4,4,3,3,3,4,9,9,3,9,9,9,9,9,3,3,4,4,10,9,9,3
    
    We can see that if our sample count is 4 and below it's a 0. If the sample count is 8 and
    above it's a 1.

    Sorted
    0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,8,9,9,9,9,9,9,9,9,9,9,9,9,10,10,11

    I've sorted the data and we easily see a cutoff. If we have (8 - 4)/2 = 6, then we can
    have a threshold of kCutoff = 6. Then if the count is below 6 it's a 0, and above 6 it's
    a 1.

    But what if we use a different clock speed? Wouldn't kCutoff need to change? Yup.
    So I've implemented a crappy ckmeans algorithm to perform 1d data clustering which
    would then output a kCutoff value. This is for a k of 2.
*/
uint8_t calculated_sums[43] = {0};
uint8_t calculated_sum_of_squares[43] = {0};

uint8_t DHT22::Calculate_kCutoff()
{
    // ckmeans algorithm for k=2

    float best_cost = 99999.0f;
    uint8_t best_index = 0;

    for (int i = 0; i < kBufferSize; i++)
    {
        calculated_sums[i+1] = calculated_sums + buffer_[i];
        calculated_sum_of_squares[i+1] = calculated_sum_of_squares[i] + buffer_[i] * buffer_[i];
    }

    for (uint8_t split = 1; split < (kBufferSize - 1); split++)
    {
        float cost = Cost(0, split) + Cost(split, kBufferSize);

        if (cost < best_cost)
        {
            best_cost = cost;
            best_index = split;
        }
    }

    return (buffer_[best_index - 1] + buffer_[best_index]) * 0.5f;
}

float DHT22::Cost(uint8_t start, uint8_t end)
{
    float sum = calculated_sums[end] - calculated_sums[start];
    float sum_of_squares = calculated_sum_of_squares[end] - calculated_sum_of_squares[start];
    uint8_t n = end - start;

    if (n <= 1)
        return 0.0f;

    return sum_of_squares - (sum * sum)/n;
}