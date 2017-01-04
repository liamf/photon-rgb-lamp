
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

#include "pulse.h"
#include "admin.h"

LightPulser::LightPulser(void) : pulseEnabled(false)
{
    // Pulse level is in percentage: 100% is fully at the selected level
    currentLevel   = 100;
    pulseDirection = -1;    // Decrememting in increments of 1%
}
 
void LightPulser::onTimeout(void)
{
    if( pulseEnabled)
    {
        currentLevel = currentLevel + pulseDirection;
        
        if( currentLevel < 0 ) currentLevel  = 0;
        if( currentLevel > 100) currentLevel = 100;
        
        uint32_t r = (maxRedLevel * currentLevel) / 100;
        uint32_t g = (maxGreenLevel * currentLevel) / 100;
        uint32_t b = (maxBlueLevel * currentLevel) / 100;
        
        lamp.setColour(r,g,b);
        
        if( currentLevel == 0) pulseDirection = 1;
        
        if( currentLevel == 100) pulseDirection = -1;
    }
}

// Turn on or off the fading function.
// Actual lamp fading done by a s/w timer
// Peculiar things will happen if you have pulsing enabled and try to control the lamp colour as well 

void LightPulser::enablePulse(bool newState)
{
    if( newState == true)
    {
        currentLevel = 100;
        pulseDirection = -1;
        
        COLOUR col = lamp.getColour();
        
        maxRedLevel   = col.r;
        maxGreenLevel = col.g;
        maxBlueLevel  = col.b;
        
        pulseEnabled = true;
    }
    else
    {
        pulseEnabled = false;
        
        currentLevel     = 100;
        pulseDirection = -1;
        
        lamp.setColour(maxRedLevel, maxGreenLevel, maxBlueLevel);
    }
}

// Control pulsing of the light ... doesn't mix well with repeatedly setting the colour
// you need to turn off pulse mode before changing the colour
int PulseLamp(String command)
{
    int numArgs;
    int retval = -1;
    
    command.trim();
    command.toUpperCase();
    
    String pulseCommand[4];
    numArgs = splitStringToArray(command, pulseCommand);
    
    if( debugEnabled) {
        Serial.printf("Pulse command received: ");
        
        for( int i=0; i < numArgs; i++) {
            Serial.printf("%s ",pulseCommand[i].c_str());
        }
        Serial.printf("\n");
    }
    
    String action = pulseCommand[0];
    if( action == "ON")
    {
        lightPulse.enablePulse(true);
        retval = 0;
    }
    else if( action == "OFF")
    {
        lightPulse.enablePulse(false);
        retval = 0;
    }
    else if (action == "PERIOD")
    {
        return ChangePulsePeriod(pulseCommand[1]);
    }
    
    return retval;    
}

// We allow periods from 0.5-1000 seconds: anything outside that time is not allowed
// Except for period of 0: that turns off the pulse
int ChangePulsePeriod(String command)
{
    float newPeriod = command.toFloat();
    
    if( newPeriod < 0 || newPeriod > 1000 )
    {
        // Reject, do nothing
        return -1;
    }
    
    // Period of 0 == disable pulse
    if( newPeriod == 0 )
    {
        lightPulse.enablePulse(false);
    }
    else
    {
        if( newPeriod < 0.5) newPeriod = 0.5;
        
        // Anything else: compute the new interval. Since we pulse in 1% intervals, this is just 10 x number of seconds to give is mSec
        fader.changePeriod( (int)(10 * newPeriod));
    }
    
    return 0;
}
