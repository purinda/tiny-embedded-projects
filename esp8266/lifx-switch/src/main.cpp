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
#include "../../../config/parameters.h"
#include <lifx.h>
extern "C" {
    #include "user_interface.h"
}

#define BOUNCE_DURATION 20;
volatile unsigned long bounceTime = 0;

uint8_t buttonToggled = 0;
uint8_t buttonTimeout = 0;
uint8_t interuptPin   = 0;

Lifx *lx = new Lifx();

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
    byte macLxBulb[6] = {0xD0, 0x73, 0xD5, 0x26, 0xB8, 0x4D};
    IPAddress lxBulb;
    lxBulb.fromString("10.10.0.51");

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

    // Setup Interupts: The switch which would turn on/off lifx bulb
    pinMode(0, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(0), isr_p0, CHANGE);
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