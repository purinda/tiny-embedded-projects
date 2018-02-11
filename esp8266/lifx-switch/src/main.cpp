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
#include <lifx.h>
#include "../../../config/parameters.h"
extern "C" {
#include "user_interface.h"
}

#define BOUNCE_DURATION 20;
volatile unsigned long bounceTime = 0;

byte        lxMacAddr[]   = {0xD0, 0x73, 0xD5, 0x26, 0xB8, 0x4D};
const char* lxIpAddr      = "10.10.0.60";
uint8_t     buttonToggled = 0;
uint8_t     buttonTimeout = 0;

// Pin D5 is used as the switch
uint8_t interuptPin = 14;

Lifx* lx = new Lifx();

/**
 *   @brief  This function gets called when the interruptPin is "changed".
 *
 *   @return void
 */
void isr_p0() {
    // Debounce functionality handled here.
    if (millis() > bounceTime) {
        bounceTime = millis() + BOUNCE_DURATION; // set whatever bounce time in ms is appropriate
    } else {
        return;
    }

    buttonToggled = !buttonToggled;

    if (buttonToggled == 0) {
        lx->setPower(0);
    } else {
        lx->setPower(65534);
    }

    /* Button watching */
    char output[8];
    sprintf(output, "Toggled %d", buttonToggled);
    Serial.println(output);
    Serial.println("");
}

/**
 *   @brief  Arduino "setup" method or boot method. Initialisation
 *           of the application should be done here.
 *
 *   @return void
 */
void setup() {
    IPAddress ipLxBulb;
    ipLxBulb.fromString(lxIpAddr);

    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
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

    // Setup the bulb which will be controlled using the switch
    lx->setBulb(0, lxMacAddr, ipLxBulb);

    // Setup Interupts: The switch which would turn on/off lifx bulb
    pinMode(interuptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interuptPin), isr_p0, CHANGE);
}

void loop() {
    // Just print system info periodically
    Serial.println();
    Serial.println("System Info.");

    uint32_t freeHeap = system_get_free_heap_size();
    Serial.print("Heap Free (bytes): ");
    Serial.println(freeHeap);

    // 5s sleep
    delay(5000);
}