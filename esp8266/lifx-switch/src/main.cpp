/**
 *  @file    main.cpp
 *  @author  Purinda Gunasekara (purinda@gmail.com)
 *  @date    20/01/2018
 *
 *  @brief Lifx Bulb Controller
 *
 *  @section DESCRIPTION
 *
 *  This is an application for controlling the Lifx bulbs using
 *  the LAN api.
 *
 *  Currently supports switching on/off. The code has measures to
 *  prevent physical switch bounces that could cause ISR to fire
 *  irradically.
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "../../config/global_config.h"
#include "lifx.h"

#define BOUNCE_DURATION 20;
volatile unsigned long bounceTime = 0;

byte      lxDevices[MAX_LX_DEVICES][SIZE_OF_MAC];
IPAddress lxDevicesAddr[MAX_LX_DEVICES];

IPAddress bcastAddr(255, 255, 255, 255);
WiFiUDP   UDP;

uint8_t buttonToggled = 0;
uint8_t buttonTimeout = 0;
uint8_t interuptPin   = 0;

/**
 *   @brief  This function gets called when the interruptPin is "changed".
 *
 *   @return void
 */
void isr_p0() {
    // it ignores presses that occur in intervals less then the bounce time
    if (millis() > bounceTime) {
        // Your code here to handle new button press ?
        bounceTime = millis() + BOUNCE_DURATION; // set whatever bounce time in ms is appropriate
    } else {
        return;
    }

    buttonToggled = !buttonToggled;

    if (buttonToggled == 0) {
        lxPower(0);
    } else {
        lxPower(65534);
    }

    /* Button watching */
    char output[8];
    sprintf(output, "Toggled %d", buttonToggled);
    Serial.println(output);
    Serial.println("");
}

/**
 *   @brief  Reset ISR timeout.
 *
 *   @return void
 */
void isr_timeout() {
    buttonTimeout = 1;
}

/**
 *   @brief  Arduino "setup" method or boot method. Initialisation
 *           of the application should be done here.
 *
 *   @return void
 */
void setup() {
    uint8_t zeros[SIZE_OF_MAC] = {0};
    for (int i = 0; i < MAX_LX_DEVICES; i++) {
        memcpy(lxDevices[i], zeros, SIZE_OF_MAC);
    }

    // Setup LIFX Master Bedroom
    lxDevices[0][0]     = 0xD0;
    lxDevices[0][1]     = 0x73;
    lxDevices[0][2]     = 0xD5;
    lxDevices[0][3]     = 0x26;
    lxDevices[0][4]     = 0xB8;
    lxDevices[0][5]     = 0x4D;
    lxDevicesAddr[0][0] = 10;
    lxDevicesAddr[0][1] = 10;
    lxDevicesAddr[0][2] = 0;
    lxDevicesAddr[0][3] = 51;

    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    bcastAddr    = WiFi.localIP();
    bcastAddr[3] = 255;

    /* Setup Interupts */
    pinMode(0, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(0), isr_p0, CHANGE);
}


void loop() {
    // intentionally ignored due to interrupts used
}