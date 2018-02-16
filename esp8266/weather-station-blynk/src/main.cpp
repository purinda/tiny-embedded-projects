/**
 * Weather Station Application that uses Blynk platform for reporting.
 * Supports reading of
 *  - Temperature
 *  - Humidity
 *  - Pressure
 *
 * BME280 Bosch component is used alongside ESP8255
 * References
 *  - https://github.com/coding-with-craftsmen/esp8266-BME280
 *  - https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide
 *  -
 * http://cactus.io/hookups/sensors/barometric/bme280/hookup-arduino-to-bme280-barometric-pressure-sensor
 */

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ota.h>
#include <BlynkConfig.h>
#include <CommonConfig.h>

#define BLYNK_PRINT Serial
#define BLYNK_DEBUG

#define SEALEVELPRESSURE_HPA (1013.25) // 1 Standard Atmosphere
#define BME280_I2C_ADDR 0x76           // BME280 I2C address
#define READ_FREQ 60000                // Collect sensor data every 1min and publish to Blynk

Adafruit_BME280 bme; // I2C
WiFiClient      client;
OTA             ota;

long  lastMsg  = 0;
float temp     = 0.0;
float hum      = 0.0;
float pressure = 0.0;
float alt      = 0.0;
float diff     = 1.0;

// I2C pin config
const uint8_t i2c_scl = 5;
const uint8_t i2c_sda = 4;

void setup() {
    ota.setup();

    // Setup I2C interface
    Wire.begin(i2c_sda, i2c_scl);

    pinMode(LED_BUILTIN, OUTPUT);
    delay(10);

    if (!bme.begin(BME280_I2C_ADDR, &Wire)) {
        Serial.print("> Ooops, no BME280 detected ... Check your wiring or I2C ADDR!");
        while (1);
    } else {
        Serial.println("> BME280 ready. Booting..");
    }

    Serial.println('> Setting up WiFi..');
    Serial.print("\tHardware Address: ");
    Serial.println(WiFi.macAddress());

    Serial.println("\tWiFi connected");
    Serial.print("\tIP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("> Configuring Blynk ");
    Blynk.begin(BlynkConfig::blynkAuth, CommonConfig::ssid, CommonConfig::password);
    bool result = Blynk.connect();
    if (result) {
        Serial.println("\tBlynk connected..");
    }

    Serial.println();
}

void loop() {
    ota.run();
    Blynk.run();
    long now = millis();

    if (!lastMsg || now - lastMsg > READ_FREQ) {
        lastMsg = now;

        temp     = bme.readTemperature();
        hum      = bme.readHumidity();
        pressure = bme.readPressure();
        alt      = bme.readAltitude(SEALEVELPRESSURE_HPA);

        Blynk.virtualWrite(V1, temp);
        Blynk.virtualWrite(V2, hum);
        Blynk.virtualWrite(V3, pressure);
        Blynk.virtualWrite(V4, alt);
        
        Serial.println("> Reading sensor data and publishing to Blynk.");
    }
}