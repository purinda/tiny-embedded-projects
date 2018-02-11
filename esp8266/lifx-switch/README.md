# Physical Switch for Lifx Bulbs

This is a small application using ESP8266 and Arduino framework to control a **single** Lifx bulb. 

The purpose of the application is to address the pain point in using Lifx bulbs which is, 
if you switch off a bulb from the power switch they cannot be turned on
using the Lifx mobile app or voice commands (through Google Assistant or Amazon Alexa). 
The application provides a physical switch that toggles a designated Lifx bulb.

## Setup & Connections

1. Configure `../../config/parameters.h` with your WiFi details.
2. Connect a switch between `D5` pin and `GND`, this can be configured using `interruptPin` variable.
3. Change `lxMacAddr` and `lxIpAddr` variables to fir the MAC and IP of your Lifx bulb that needs to be controlled.
4. Compile and push the code to your ESP8266 module and play.
