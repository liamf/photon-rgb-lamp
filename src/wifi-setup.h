/*
 * This app controls the "ambient orb" RGB light clone and exposes a few simple control points to the particle cloud
 *
 * Liam Friel
 *
 * Copyright (c) 2015 guthub user @mebrunet who contributed the softAP pages to the Spark community pages
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
 
#ifndef wifi_setup_h
#define wifi_setup_h

// This is required for some Photon functions to work properly
#pragma SPARK_NO_PREPROCESSOR

#include "Particle.h"
#include "softap_http.h"

struct Page
{
    const char* url;
    const char* mime_type;
    const char* data;
};

#define CONNECTED   0
#define CONNECTING  1
#define LISTENING   2

// Hardcoded page definitions 
extern const char index_html[];
extern const char rsa_js[];
extern const char style_css[];
extern const char rng_js[];
extern const char jsbn_2_js[];
extern const char jsbn_1_js[];
extern const char script_js[];
extern const char prng4_js[];

void setupWiFiPage(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved);


class Connection
{
    public:
        Connection(void);
        
        bool isConnected(void);
        bool updateConnectionStatus(void);
        
        int getCloudRecoveryState(void);
        void setCloudRecoveryState(int);
        
        unsigned long getLastStateChange(void);
        unsigned long mSecSinceLastStateChange(void);
        
    private:
        bool currentConnectionStatus; 
        int  cloudRecoveryState;
        unsigned long lastStateChange;
};


#endif
