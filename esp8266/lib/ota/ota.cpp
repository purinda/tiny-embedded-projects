#include "ota.h"

/**
 * Configures the remote OTA update mode.
 *
 * @return void
 */
void OTA::setup() {
    Serial.begin(CommonConfig::serialBaud);
    Serial.println("Booting..");

    WiFi.mode(WIFI_STA);
    WiFi.begin(CommonConfig::ssid, CommonConfig::password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");

        delay(5000);
        ESP.restart();
    }

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(CommonConfig::nodeName);

    ArduinoOTA.onStart([]() { Serial.println("Start"); });
    ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
            Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("OTA Mode Ready.");
    Serial.print("Node Address: ");
    Serial.println(WiFi.localIP());
}

/**
 * Place this function under Arduino loop() method to keep the OTA mode
 * running.
 *
 * @return void
 */
void OTA::run() {
    ArduinoOTA.handle();
}