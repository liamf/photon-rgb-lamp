/*
 * This app controls the "ambient orb" RGB light clone and exposes a few simple control points to the particle cloud
 *
 * Liam Friel
 *
 * Copyright (c) 2016/2017 Liam Friel
 *
 * Permission is hereby granted, free of charge, 
 * to any person obtaining a copy of this software and 
 * associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom 
 * the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice 
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef light_h
#define light_h

#include <stdint.h>

#include "Particle.h"

extern int redLevel;
extern int greenLevel;
extern int blueLevel;
extern int bitsPerPixel;

typedef struct 
{
    uint32_t r;
    uint32_t g;
    uint32_t b;
} COLOUR;

// Generic lamp handler, exposed to the cloud
int LampControl(String command);
int PulseLamp(String command);

int SetLampColour(String arg1, String arg2, String arg3);
int SetLampColourFromRamp(String arg1, String arg2, String arg3);
int SetLampColourFromSpectrum(String arg1, String arg2, String arg3);


// 
class Light
{
  
    public:
        Light( int rPin, int gPin, int bPin);
        
        COLOUR setColour( uint32_t red, uint32_t green, uint32_t blue);
        COLOUR restoreColour(void);
        COLOUR set8BitColour( uint8_t red, uint8_t green, uint8_t blue);

        COLOUR getColour(void);
        
        // Utility functions
        void setRed( uint32_t red);
        void setGreen( uint32_t green);
        void setBlue( uint32_t blue);
        
        void setColourResolution(int bits);
        int  getColourResolution(void);
        
        bool lampControlEnabled(void);
        void setExternalLampControl(bool autoMode);
        
        void setBrightnessLevel(int level);
        int  getBrightnessLevel(void);
        
        void   rapidColourRamp(void);
        
        void   setRestoreColour(void); 
        COLOUR colourRampFromRange(float value, float minValue, float maxValue);
        COLOUR visibleColourFromRange(float value, float minValue, float maxValue);
        
    private:
        int redPin;
        int greenPin;
        int bluePin;
        int brightnessLevel;
        
        bool lampControlIsEnabled;

        COLOUR currentColour;
        COLOUR savedColour;
};

#endif