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
 
#include "admin.h"

bool debugEnabled = false;

// Network control command
int AdminHandler(String command)
{
    int numArgs;
    
    command.trim();
    command.toUpperCase();
    
    String adminCommand[8];
    numArgs = splitStringToArray(command, adminCommand);
    
    if( debugEnabled) {
        Serial.printf("Admin command received: ");
        
        for( int i=0; i < numArgs; i++) {
            Serial.printf("%s ",adminCommand[i].c_str());
        }
        Serial.printf("\n");
    }
    
    String action = adminCommand[0];
    
    // Now just hand off to the handlers
    if( action == "CLEAR")
    {
        return ForgetNetworkConfiguration();
    }
    else if (action == "LIST")
    {
        return PrintKnownNetworks();
    }
    else if (action == "ADD")
    {
        return AddNetworkCredentials( adminCommand );
    }
    else if (action == "SERIAL")
    {
        return EnableUSBSerial(adminCommand[1]);
    }
    else if (action == "DEBUG")
    {
        return EnableDebug(adminCommand[1]);
    }
    else if (action == "LED")
    {
        return EnableLEDControl(adminCommand[1]);
    }
    
    return -1;
}

// Handle adding network credentials via API
int AddNetworkCredentials( String *command )
{
    int retval = -1;
    if( command[0] == "ADD")
    {
        retval = 0;
        if( command[1] == "UNSEC")
        {
            WiFi.setCredentials(command[2]);    
        }
        else if (command[1] == "WEP")
        {
            WiFi.setCredentials( command[2], command[3], WEP);
        }
        else if (command[1] == "WPA2")
        {
            // Adding a WPA2 network. We have the option of specifying the cypher
            if( command[4] == "AES")
            {
                WiFi.setCredentials(command[2], command[3], WLAN_CIPHER_AES);
            }
            else if (command[4] == "TKIP")
            {
                WiFi.setCredentials(command[2], command[3], WLAN_CIPHER_TKIP);    
            }
            else if (command[4] == "AES_TKIP")
            {
                WiFi.setCredentials(command[2], command[3], WLAN_CIPHER_AES_TKIP);
            }
            else
            {
                WiFi.setCredentials(command[2], command[3]);
            }
        }
    }
    
    return retval;
}

// Can be handy for troubleshooting ... 
int PrintKnownNetworks(void)
{
    WiFiAccessPoint ap[5];
    int found = WiFi.getCredentials(ap, 5);
    for (int i = 0; i < found; i++) 
    {
        Serial.print("ssid: ");
        Serial.println(ap[i].ssid);
        // security is one of WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA, WLAN_SEC_WPA2
        Serial.print("security: ");
        Serial.println(ap[i].security);
        // cipher is one of WLAN_CIPHER_AES, WLAN_CIPHER_TKIP
        Serial.print("cipher: ");
        Serial.println(ap[i].cipher);
    }
    
    return found;
}

// Clears all credentials. Does not immediately put core offline, but next restart will put it into listening mode
// until new credentials are set via SoftAP or USB interface
int ForgetNetworkConfiguration(void)
{
    WiFiAccessPoint ap[5];
    int found = WiFi.getCredentials(ap, 5);
    WiFi.clearCredentials();
    return found;
}

// Turn on/off serial. Useful for debug (if you have a USB cable connected ...)
int EnableUSBSerial(String command)
{
    int retval = -1;
    
    if( command == "ON")
    {
        Serial.begin();
        retval = 0;
    }
    else if( command == "OFF")
    {
        Serial.end();
        retval = 0;
    }
    
    return retval;
}

// Printouts
int EnableDebug(String command)
{
    int retval = -1;
    
    if( command == "ON")
    {
        debugEnabled = true;
        Serial.printf("Debug ON\n");
        retval = 0;
    }
    else if( command == "OFF")
    {
        debugEnabled = false;
        Serial.printf("Debug OFF\n");
        retval = 0;
    }
    
    return retval;
}

int EnableLEDControl(String command)
{
    int retval = -1;
    
    if(command == "MANUAL")
    {
        lamp.setExternalLampControl(true);
        retval = 0;
    }
    else if( command == "AUTO")
    {
        lamp.setExternalLampControl(false);
        retval = 1;
    }
    
    return retval;
}

// Split up a string into words (== arguments), assumed separated by spaces
int splitStringToArray(String arguments, String *target)
{
    int numArgs = 0;
    int beginIdx = 0;
    int idx = arguments.indexOf(" ");

    while (idx != -1) {
	    String arg = arguments.substring(beginIdx, idx);
	    target[numArgs] = arg;

	    beginIdx = idx + 1;
	    idx = arguments.indexOf(" ", beginIdx);
	    ++numArgs;
    }

    // Single or last parameter
    String lastArg = arguments.substring(beginIdx);
    target[numArgs] = lastArg;
    
    return numArgs + 1;
}
