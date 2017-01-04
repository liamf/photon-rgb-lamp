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
 
 // This #include statement was automatically added by the Particle IDE.
// Local libraries
#include "light.h"
#include "wifi-setup.h"
#include "admin.h"
#include "pulse.h"


// Number of seconds to show the pusling status LED before turning control over to the 
#define CONNECTED_INDICATION_TIME   10

// Number of seconds that the Photon stays in LISTENING or CONNECTING state, while attempting to connect (or allow the core to connect)
// Has to be reasonable long to allow you to access and enter the WiFi credentials
#define RECOVERY_CYCLE_PERIOD 120

// Enable threading for the application
// This allows us to do things like monitor connection status
SYSTEM_THREAD(ENABLED);

// This overrides the default system mode
// Required to make the lamp colour mirror the on-board LED while connecting, otherwise
// the setup() and loop() functions do not run until the board is connected to the cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

// The PWM pins which control Red, Green and Blue colours
// Note: this works on Photon only, not on earlier Spark cores
// Suitable Spark choices are A0, A1, A4
// This sketch won't build for a Spark Core anyway: older CC3000 modules had no AP mode, so the WiFi setup via SoftAP won't work

const int redLED = D0;
const int greenLED = D1;
const int blueLED = D2;

// Track our connection status
Connection cloudConnection;

// bool isConnectedToCloud = false;

// Create our lamp control class
Light lamp(redLED, greenLED, blueLED);
LightPulser lightPulse;

Timer fader(50, &LightPulser::onTimeout, lightPulse);

// Register our variables and functions and kick off cloud connectivity
void setup() {


    // Setup the PIN modes and the colour resolution
    lamp.setColourResolution(12);
    
    // Turn off lamp control (lamp colour mirrors onboard LED)
    lamp.setExternalLampControl(false);

    // Now register the handler to allow the lamp colour mirror the onboard LED
    RGB.onChange(ledChangeHandler);
    
    // Register the lamp colour variables. Just R,G,B levels, and bits per pixel
    Particle.variable("red", redLevel);
    Particle.variable("green", greenLevel);
    Particle.variable("blue", blueLevel);
    Particle.variable("bpp", bitsPerPixel);
    
    // These are debug functions for now
    Particle.function("colour",LampControl);
    Particle.function("pulse", PulseLamp);
    
    // Admin comments
    Particle.function("admin", AdminHandler);
    
    // Register the WiFi configuration pages
    STARTUP(softap_set_application_page_handler(setupWiFiPage, nullptr));
    
    // Set the initial cloud recovery state to CONNECTED (we know it isn't CONNECTED yet, but it will get reset momentarily by the normal recovery process)
    cloudConnection.setCloudRecoveryState(CONNECTED);
    
    // Setup the light "pulser" timeer callback
    fader.start();
    
    // Initiate the connection to the Particle core
    Particle.connect();
    
}

// Status monitoring loop
void loop() {
    bool currentConnectionState;
    
    currentConnectionState = cloudConnection.updateConnectionStatus();
    
    // Catch the case where we have held down the SETUP button for >5 seconds and forced the core into LISTENING mode
    if( System.buttonPushed() > 5000 ) {
        cloudConnection.setCloudRecoveryState(LISTENING);    
    }
    
    // If we are currently connected, and have been connected for > 10 and < 15 seconds, then
    // ... allow LED control
    // ... turn off the lamp to await remote control
    // ... this is basically normal mode
    
    if( currentConnectionState && ( (cloudConnection.mSecSinceLastStateChange() > (CONNECTED_INDICATION_TIME * 1000)) && ( cloudConnection.mSecSinceLastStateChange() < ((CONNECTED_INDICATION_TIME + 5) * 1000) )))
    {
        if( debugEnabled) {
            Serial.printf("Connected: going to turn on external lamp control\n");
        }
        
        lamp.setExternalLampControl(true);
        lamp.rapidColourRamp();
        delay(50);
        lamp.setColour(0,0,0);
    }
    else 
    {

        // If we're not connected, and we have not been connected for > 60 seconds, then we attempt our recovery cycle 
        if( !currentConnectionState )
        {
            if( debugEnabled) {
                Serial.printf("Not connected ... mSec since change %d\n", cloudConnection.mSecSinceLastStateChange());
            }
            
            // If we've been disconnected for > half the recovery time seconds, turn back on lamp status mirroring
            if(lamp.lampControlEnabled() && cloudConnection.mSecSinceLastStateChange() > (RECOVERY_CYCLE_PERIOD/2)*1000 )
            {
                if( debugEnabled) {
                    Serial.printf("Mirroring status LED\n");
                }
                
                // Lamp colour indicates the connection status again, to aid reconnection
                lamp.setExternalLampControl(false);
            }
            
            // Our recovery is 120 seconds of attempting to reconneect, 120 seconds of listening, repeating forever ... 
            if( cloudConnection.getCloudRecoveryState() == CONNECTING)
            {
                if( debugEnabled) {
                    Serial.printf("In CONNECTING state ..");
                }
                
                if( cloudConnection.mSecSinceLastStateChange() > (RECOVERY_CYCLE_PERIOD*1000) || WiFi.listening() )
                {
                    if( debugEnabled) {
                        Serial.printf("Setting LISTENING Mode\n");
                    }
                    
                    cloudConnection.setCloudRecoveryState(LISTENING);
                    WiFi.listen();
                }
            }
            else if (cloudConnection.getCloudRecoveryState() == LISTENING)
            {
                if( debugEnabled) {
                    Serial.printf("In LISTENING state ..");
                }
                if( cloudConnection.mSecSinceLastStateChange() > (RECOVERY_CYCLE_PERIOD*1000) && WiFi.hasCredentials() )
                {
                    if( debugEnabled) {
                        Serial.printf("Setting CONNECTING state\n");
                    }
                    cloudConnection.setCloudRecoveryState(CONNECTING);
                    WiFi.listen(false);
                }
            }
        }

    }
    // Wait for a second and check again
    delay(1000);
}



// Mirrors the LED status connection to the lamp, if allowed to do so ... 
void ledChangeHandler(uint8_t r, uint8_t g, uint8_t b)
{
    COLOUR newColour;
    
    if( !lamp.lampControlEnabled() ) {
        newColour = lamp.set8BitColour(r,g,b);
        
        redLevel    = newColour.r;
        greenLevel  = newColour.g;
        blueLevel   = newColour.b;
    }
}



