# photon-rgb-lamp
Code for the Particle Photon, to control an RGB lamp
Assumes that the RGB lamp is PWM controlled

You can power an arbitrarily powerful RGB LED or LED array like this, with the correct circuit design.

Exposes a number of APIs to the Particle cloud to allow you to control your lamp.

##REST endpoints:  

**/v1/devices/_deviceid_/colour**  
**/v1/devices/_deviceid_/pulse**  
**/v1/devices/_deviceid_/admin**  

The Particle REST API accepts an optional single argument, assumed a string of up to 63 characters max, for each REST endpoint.
There follows a short description of each. 

Each of these in turn:  
###/v1/devices/_deviceid_/colour  

Sets the colour of the lamp.  
Either the arg "SET" followed by the word RED, GREEN or BLUE with an integer number
This will set the RED, GREEN or BLUE colour only leaving the others unchanged.

or SET followed by 3 integers, assumed to be the RED, GREEN and BLUE colours.

The range for colour setting is set by the bits per pixel (default 12 bits, so 0 - 4095)

arg to pass:  
**SET RED   x**  
**SET GREEN x**  
**SET BLUE  x**  
**SET r g b**  

These two functions set the RGB colour to a point on either of two built-in colour ranges, given a value, and a min/max range.
e.g. RAMP 200 0 1000 sets the RGB colour to a colour which represents the 20% of the way between 0 and 1000

**RAMP val vMin vMax**  
**SPECTRUM val vMin vMax**  

###/v1/devices/_deviceid_/pulse
"Pulses" the currently set lamp colour from on->off and back, with a default period of 5 seconds

arg to pass:  
ON  
OFF  
PERIOD x  
  
ON turns on the Pulse function (default: off)  
OFF turns it back off again  
PERIOD x sets the pulse period time, floating point number allowed (allowed range: 0.5 - 1000 seconds)  

e.g. PERIOD 0.5  
     PERIOD 10  
	 
###/v1/devices/_deviceid_/admin

Contains a number of admin commands.

arg to pass:  
CLEAR  
LIST   
SERIAL ON|OFF  
DEBUG  ON|OFF  
LED    AUTO|MANUAL  
ADD    UNSEC|WEP|WPA2 <SSID> [<PASSWORD>] [TKIP|AES|AES_TKIP]  

**CLEAR** deletes all stored WiFi credentials in the Photon. On reboot, the Photon will go into Listening mode and await credentials  
**LIST**  prints out to the USB serial port (if it is enabled, see later) the list of networks stored currently  
**SERIAL** ON or OFF turns ON or OFF the USB serial port  
**DEBUG** ON of OFF turns on or off some debug tracing to the USB serial port, if this port is enabled  
**LED** AUTO forces the lamp to follow the colour of the Photon on-board LED. This is useful if you are going to flash the Photon with new firmware  
    and want to see the progress. LED MANUAL returns to normal mode, where the lamp can be controlled via the REST API.  
  
The **ADD** command	allows you to send the core WiFi credentials via API. This is useful to setup the Photon for a network which is not currently available or is at a different location.
  
UNSEC sets up access to an Open network (SSID only required, no password)  
WEP sets up access to a WEP network, and required SSID and PASSWORD.  
WPA2 is the normal WPA2 network. SSID and PASSWORD are required. For a network which is hidden or offline, the cipher must be specified (TKIP, AES, or AES_TKIP)  

       