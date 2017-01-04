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
 
#ifndef admin_h
#define admin_h

#include "Particle.h"
#include "light.h"
#include "pulse.h"

extern bool debugEnabled;
extern Light lamp;
extern LightPulser lightPulse;
extern Timer fader;

// Generic admin handler
int AdminHandler(String command);

// Action routines
int PrintKnownNetworks(void);
int ForgetNetworkConfiguration(void);
int EnableUSBSerial(String command);
int EnableDebug(String command);
int AddNetworkCredentials(String *command);
int EnableLEDControl(String command);

// Utility to split a String into individual strings on spaces
int splitStringToArray(String arguments, String *target);

#endif
