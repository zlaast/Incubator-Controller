/*********************************************************************

    File: display.h

    Author: Z'Laast Allicock
    
    Description:
        Send text to TM1637 display

    Notes:
        TM1637 Display library from: https://github.com/avishorp/TM1637
        
        For examples on how this TM1637 library works, see:
            https://github.com/avishorp/TM1637/blob/master/TM1637Display.h
            https://github.com/avishorp/TM1637/blob/master/examples/TM1637Test/TM1637Test.ino

*********************************************************************/

#pragma once

#include <Arduino.h>
#include <TM1637Display.h>

class Display
{
    private:
        TM1637Display display;
        const uint8_t C[1] = { SEG_A | SEG_F | SEG_E | SEG_D };             // C for Celsius
        const uint8_t H[1] = { SEG_F | SEG_E | SEG_G | SEG_B | SEG_C };     // H for Humidity

        const uint8_t Err[3] = {                                            // Err for error
            SEG_A | SEG_F | SEG_G | SEG_E | SEG_D,                          // E
            SEG_E | SEG_G,                                                  // r
            SEG_E | SEG_G,                                                  // r
        };

        const uint8_t Incu[4] = {
            SEG_B | SEG_C,                                                  // I
            SEG_E | SEG_G | SEG_C,                                          // n
            SEG_G | SEG_E | SEG_D,                                          // c
            SEG_E | SEG_D | SEG_C                                           // u
        };

        const uint8_t bAtE[4] = {
            SEG_F | SEG_G | SEG_E | SEG_C | SEG_D,                          // b
            SEG_A | SEG_F | SEG_B | SEG_G | SEG_E | SEG_C,                  // A
            SEG_F | SEG_G | SEG_E | SEG_D,                                  // t
            SEG_A | SEG_F | SEG_G | SEG_E | SEG_D                           // E
        };

    public:
        Display(uint8_t CLK, uint8_t DIO): display(CLK, DIO) 
        {
            // Small welcome message that says "Incubate"
            // :)
            display.setBrightness(0x0f);
            _delay_ms(100);
            
            display.setSegments(Incu);
            _delay_ms(2000);
            Clear();

            display.setSegments(bAtE);
            _delay_ms(2000);
            Clear();
        }

        void Clear();
        void ShowError(uint8_t error);                                    // Error Code. It's whatever you choose it to be.
        void ShowHumidity(float humidity);
        void ShowTemperature(float temperature);
};