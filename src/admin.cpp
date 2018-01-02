*
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
 
#include "light.h"
#include "admin.h"
#include "math.h"
#include "application.h"

// Easy access for the current colour values. Perhaps these can be classs variables and exposed to the Cloud?

int redLevel;
int greenLevel;
int blueLevel;
int bitsPerPixel;
int powerLevel;


Light::Light(int rPin, int gPin, int bPin ) : redPin(rPin), greenPin(gPin), bluePin(bPin), brightnessLevel(100)
{
    lampControlIsEnabled = false;
    bitsPerPixel = 8;
    
    redLevel = greenLevel = blueLevel = 0;
    powerLevel = 100;
    
    savedColour.r = savedColour.g = savedColour.b = 0;
};


// Sets the colour resolution of the PWM pins (and set them to be outputs, just in case)
void Light::setColourResolution(int bits)
{
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    
    analogWriteResolution(redPin, bits);    
    analogWriteResolution(greenPin, bits);  
    analogWriteResolution(bluePin, bits);   
    
    bitsPerPixel = bits;
}

// Returns the colour resolution (in bits)
// ... so not quite the inverst of 
int Light::getColourResolution()
{
    return analogWriteResolution(redPin);
}

/*
 * Sets the actual lampc colour
 * R,G,B can take values in the range 0-<colour resolution>. These are the permitted PWM settings 
 *
 * Values are clamped if they are outside these ranges
 */
COLOUR Light::setColour(uint32_t red, uint32_t green, uint32_t blue)
{
    setRed(red);
    setGreen(green);
    setBlue(blue);
    
    return currentColour;
}

COLOUR Light::getColour(void)
{
    return currentColour;    
}

COLOUR Light::restoreColour(void)
{
    return setColour(savedColour.r, savedColour.g, savedColour.b);    
}

void Light::setRed(uint32_t red)
{
    uint32_t maxColourRange = pow(2,getColourResolution()) - 1;
    uint32_t colourToSet;
    int maxFreq = analogWriteMaxFrequency(redPin);
    
    if( red > maxColourRange ) red = maxColourRange;

    // And further correct for the maximum brightness
    colourToSet = (red * brightnessLevel) / 100;
    
    // Use half the max PWM frequency
    analogWrite(redPin, colourToSet, maxFreq / 2 );

    currentColour.r = redLevel = red;
}

void Light::setGreen(uint32_t green)
{
    uint32_t maxColourRange = pow(2,getColourResolution()) - 1;
    uint32_t colourToSet;
    int maxFreq = analogWriteMaxFrequency(greenPin);
    
    // Clamp all the colours to be within allowed ranges

    if( green > maxColourRange ) green = maxColourRange;

    // And further correct for the maximum brightness
    colourToSet = (green * brightnessLevel) / 100;
    
    // Use half the max PWM frequency
    analogWrite(greenPin, colourToSet, maxFreq / 2 );

    currentColour.g = greenLevel = green;
}


void Light::setBlue(uint32_t blue)
{
    uint32_t maxColourRange = pow(2,getColourResolution()) - 1;
    uint32_t colourToSet;
    int maxFreq = analogWriteMaxFrequency(bluePin);
    
    // Clamp all the colours to be within allowed ranges
    if( blue > maxColourRange ) blue = maxColourRange;
    
    // And further correct for the maximum brightness
    colourToSet = (blue * brightnessLevel) / 100;

    // Use half the max PWM frequency
    analogWrite(bluePin, colourToSet, maxFreq / 2 );

    currentColour.b = blueLevel = blue;
}

COLOUR Light::set8BitColour(uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t maxColourRange = pow(2,getColourResolution()) - 1;
    
    
    uint32_t cRed    = (red * maxColourRange) / 255;
    uint32_t cGreen  = (green * maxColourRange) / 255;
    uint32_t cBlue   = (blue * maxColourRange) / 255;
    
    return setColour( cRed, cGreen, cBlue);
}

bool Light::lampControlEnabled(void)
{
    return lampControlIsEnabled;
}

void Light::setExternalLampControl(bool lampControl)
{
    lampControlIsEnabled = lampControl;
}

void Light::setBrightnessLevel(int level)
{
    if(level < 1) level = 1;
    if(level > 100) level = 100;
    
    brightnessLevel = powerLevel = level;    
}

int Light::getBrightnessLevel(void)
{
    return brightnessLevel;
}

/*
   Return a RGB colour value given a scalar v in the range [vmin,vmax]
   In this case each colour component ranges from 0 (no contribution) to
   255 (fully saturated), modifications for other ranges is trivial.
   The colour is clipped at the end of the scales if v is outside
   the range [vmin,vmax]
   
   This is a standard cold => hot colour gradient from http://paulbourke.net/texture_colour/colourspace/ 

*/
COLOUR Light::colourRampFromRange(float value, float vmin, float vmax)
{
    
    COLOUR c;  
    double dv;

    int maxColour = pow(2,getColourResolution()) - 1;
    c.r = c.g = c.b = maxColour;    // Lamp ON
    
    if( debugEnabled) {
        Serial.printf("----\n");
        Serial.printf("computing colour - ramp algorithm\n");
        Serial.printf("Max: %d\n", maxColour);
        Serial.printf("Value, vMin, vMax %f %f %f\n", value, vmin, vmax);
    }
    if (value < vmin)
        value = vmin;
      
    if (value > vmax)
        value = vmax;
      
    dv = vmax - vmin;

    if (value < (vmin + 0.25 * dv)) {
        c.r = 0;
        c.g = (4 * (value - vmin) / dv) * maxColour;
    } 
    else if (value < (vmin + 0.5 * dv)) 
    {
        c.r = 0;
        c.b = maxColour + (4 * (vmin + 0.25 * dv - value) / dv) * maxColour;
    } 
    else if (value < (vmin + 0.75 * dv)) 
    {
        c.r = (4 * (value - vmin - 0.5 * dv) / dv) * maxColour;
        c.b = 0;
    } 
    else 
    {
        c.g = maxColour + (4 * (vmin + 0.75 * dv - value) / dv) * maxColour;
        c.b = 0;
    }

    if( debugEnabled) {
        Serial.printf("Red: %d\n", c.r);
        Serial.printf("Green: %d\n", c.g);
        Serial.printf("Blue: %d\n", c.b);
        Serial.printf("----------\n");        
    }
    
    return(c);
}

 /*
 * This is a "visible spectrum" gradient
 *
 * Can be used to map an point on an input scale to a colour
 * Slightly different from the cold->hot colour gradient
 *
 * https://msdn.microsoft.com/en-us/library/mt712854.aspx
  */
  

COLOUR Light::visibleColourFromRange(float value, float vmin, float vmax)
{
    
    COLOUR c;  
    double dv;
    int maxColour = pow(2,getColourResolution()) - 1;

    if( debugEnabled) {
        Serial.printf("----\n");
        Serial.printf("computing colour - spectrum algorithm\n");
        Serial.printf("Max: %d\n", maxColour);
        Serial.printf("Value, vMin, vMax %f %f %f\n", value, vmin, vmax);
    }
    
    if (value < vmin)
        value = vmin;
      
    if (value > vmax)
        value = vmax;
      
    dv = vmax - vmin;

    if (value < (vmin + 0.25 * dv)) {
        c.r = maxColour - ((4 * (value - vmin) / dv) * maxColour);
        c.g = 0;
        c.b = maxColour;
    } 
    else if (value < (vmin + 0.5 * dv)) 
    {
        c.r = 0;
        c.g = (4 * (value - vmin - 0.25 * dv) / dv) * maxColour;
        c.b = maxColour + (4 * (vmin + 0.25 * dv - value) / dv) * maxColour;
    } 
    else if (value < (vmin + 0.75 * dv)) 
    {
        c.r = (4 * (value - vmin - 0.5 * dv) / dv) * maxColour;
        c.g = maxColour;
        c.b = 0;
    } 
    else 
    {
        c.r = maxColour;
        c.g = maxColour + (4 * (vmin + 0.75 * dv - value) / dv) * maxColour;
        c.b = 0;
    }

    if( debugEnabled) {
        Serial.printf("Red: %d\n", c.r);
        Serial.printf("Green: %d\n", c.g);
        Serial.printf("Blue: %d\n", c.b);
        Serial.printf("----------\n");        
    }
    
    return(c);
}

// Rapidly cycle the Lamp through the colour spectrum: a sort of "hello" message
// Does this after connecting and before powering down the lamp and waiting for a command
void Light::rapidColourRamp(void)
{
    float value;
    COLOUR colour;
    
    for( value = 0; value < 1024; delay(10), value++)
    {
        colour = colourRampFromRange(value,0,1024);
        setColour(colour.r, colour.g, colour.b);
    }
}

void Light::setRestoreColour(void)
{
    savedColour = currentColour;    
}

// Exposed Lamp control command
int LampControl(String command)
{
    int numArgs;
    int retVal = -1;
    
    command.trim();
    command.toUpperCase();
    
    String lampCommand[8];
    numArgs = splitStringToArray(command, lampCommand);
    
    if( debugEnabled) {
        Serial.printf("Colour command received: ");
        
        for( int i=0; i < numArgs; i++) {
            Serial.printf("%s ",lampCommand[i].c_str());
        }
        Serial.printf("\n");
    }
    
    String action = lampCommand[0];
    
    // Now just hand off to the handlers
    // However we have to remembed the colour, so that we can reset it
    
    if( action == "SET")
    {
        retVal = SetLampColour(lampCommand[1], lampCommand[2], lampCommand[3]);
        lamp.setRestoreColour();
    }
    else if (action == "RAMP")
    {
        retVal = SetLampColourFromRamp(lampCommand[1], lampCommand[2], lampCommand[3]);
        lamp.setRestoreColour();
    }
    else if (action == "SPECTRUM")
    {
        retVal = SetLampColourFromSpectrum( lampCommand[1], lampCommand[2], lampCommand[3] );
        lamp.setRestoreColour();
    }
    else if (action == "LEVEL")
    {
        retVal = SetLampMaximumBrightness(lampCommand[1], lampCommand[2]);
    }
    else
    {
        if (debugEnabled)
        {
            Serial.printf("That is not a valid command. Ignored");
        }
    }


    return retVal;
}

int SetLampColour(String arg1, String arg2, String arg3)
{
    uint32_t r, g, b;
    
    int retval = -1;
    if( arg1 == "RED")
    {
        r = arg2.toInt();
        lamp.setRed(r);
        retval = r;
    }
    else if (arg1 == "GREEN")
    {
        g = arg2.toInt();
        lamp.setGreen(g);
        retval = g;
    }
    else if (arg1 == "BLUE")
    {
        b = arg2.toInt();
        lamp.setBlue(b);
        retval = b;
    }
    else
    {
        // Assume that the three arguments are R, G, B integer values
        // If the values are not valid, we just get 0 
        r = arg1.toInt();
        g = arg2.toInt();
        b = arg3.toInt();
        lamp.setColour(r,g,b);
        retval = 0;
    }

    return retval;    
}

// Set the maximum brightness level (for Alexa dimming)
int SetLampMaximumBrightness(String arg1, String arg2)
{
    int retval = 0;
    int level, dim;
    
    if( arg1 == "SET")
    {
        level = arg2.toInt();
        lamp.setBrightnessLevel(level);
    }
    else if (arg1 == "DIM")
    {
        dim = arg2.toInt();
        level = lamp.getBrightnessLevel();
        level += dim;
        lamp.setBrightnessLevel(level);
    }
    else
    {
        // Just assume it is a straight set command
        level = arg1.toInt();
        lamp.setBrightnessLevel(level);        
    }
    
    // Reset the colour based on this dimming level
    lamp.restoreColour();
    
    return retval;
    
}
// Just use integers for colour values and range passed in ... 
int SetLampColourFromRamp(String arg1, String arg2, String arg3)
{
    float v, vmin, vmax;
    
    v = (float)arg1.toInt();
    vmin = (float)arg2.toInt();
    vmax = (float)arg3.toInt();
    
    COLOUR col = lamp.colourRampFromRange(v, vmin, vmax);
    
    lamp.setColour(col.r, col.g, col.b);
    
    return 0;
    
}


int SetLampColourFromSpectrum(String arg1, String arg2, String arg3)
{
    float v, vmin, vmax;
    
    v = (float)arg1.toInt();
    vmin = (float)arg2.toInt();
    vmax = (float)arg3.toInt();
    
    COLOUR col = lamp.visibleColourFromRange(v, vmin, vmax);
    
    lamp.setColour(col.r, col.g, col.b);
    
    return 0;    
}
