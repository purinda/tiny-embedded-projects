#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "../../../config/parameters.h"

/**
 * Weather Station Application
 * Supports reading and reporting of
 *  - Temperature
 *  - Humidity
 *
 * BME280 Bosch component is used alongside ESP8255
 * References
 *  - https://github.com/coding-with-craftsmen/esp8266-BME280
 *  - https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide
 *  -
 * http://cactus.io/hookups/sensors/barometric/bme280/hookup-arduino-to-bme280-barometric-pressure-sensor
 */


#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_I2C_ADDR 0x76
Adafruit_BME280 bme; // I2C

WiFiClient client;
long       lastMsg = 0;
float      temp    = 0.0;
float      hum     = 0.0;
float      diff    = 1.0;

// I2C pin config
const uint8_t  i2c_scl = 5;
const uint8_t  i2c_sda = 4;

void setup_wifi() {
    digitalWrite(LED_BUILTIN, LOW);

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
    // Setup I2C interface
    Wire.begin(i2c_sda, i2c_scl);

    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    delay(10);

    if (!bme.begin(BME280_I2C_ADDR, &Wire)) {
        Serial.print("Ooops, no BME280 detected ... Check your wiring or I2C ADDR!");
        while (1);
    } else {
        Serial.println("BME280 ready.");
    }

    // Connect to WiFi access point.
    Serial.println();
    Serial.print("Wireless MAC: ");
    Serial.println(WiFi.macAddress());

    int attempt = 0;
    setup_wifi();

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        if (++attempt % 20 == 0) {
            setup_wifi();
            Serial.printf("\nWIFI Status: %d", WiFi.status());
        }
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(10000);
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
    return !isnan(newValue) && (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void loop() {
    long now = millis();

    if (now - lastMsg > 1000) {
        lastMsg       = now;
        float newTemp = bme.readTemperature();
        float newHum  = bme.readHumidity();
    }
}