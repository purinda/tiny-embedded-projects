#ifndef OTA_H
#define OTA_H

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <CommonConfig.h>

class OTA {
    public:
        void setup();
        void run();
};

#endif