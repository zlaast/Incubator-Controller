#include "display.h"


void Display::Clear()
{
    display.clear();
}


void Display::ShowError(uint8_t error)
{
    Clear();
    display.setSegments(Err, 3, 0);                         // "Err", 3 digits modified, 1st position
    display.showNumberDec(error, false, 1, 3);              // Error number, no leading zeroes, 1 digit modified, 4th position
}


void Display::ShowHumidity(float humidity)
{
    Clear();
    int number = humidity*10;
    display.showNumberDecEx(number, 64, false, 3, 0);       // Humidity, 00.0(0b01000000)(64), no leading zeroes, 3 digits modified, 1st position
    display.setSegments(H, 1, 3);                           // Letter 'C', 1 digit modified, 4th position
}


void Display::ShowTemperature(float temperature)
{
    Clear();
    int number = temperature*10;
    display.showNumberDecEx(number, 64, false, 3, 0);       // Temperature, 00.0(0b01000000)(64), no leading zeroes, 3 digits modified, 1st position
    display.setSegments(C, 1, 3);                           // Letter 'H', 1 digit modified, 4th position
}