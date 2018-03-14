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
#include <BlynkConfig.h>
#include <BlynkSimpleEsp8266.h>
#include <CommonConfig.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ota.h>

#define BLYNK_PRINT Serial
#define BLYNK_DEBUG

#define SEALEVELPRESSURE_HPA 1013.25 // 1 Standard Atmosphere
#define TEMPERATURE_OFFSET 0         // Use this variable if the sensor seems impacted by external factors.
#define BME280_I2C_ADDR 0x76         // BME280 I2C address

#define SLEEP_DURATION 120e6 // Sleep for 2mins
#define WAKE_DURATION 30000  // Wake for 30 seconds after repoting data (lets OTA updates, etc)
#define PUBLISH_INTERVAL 30000  // Publish to Blynk every 30s

Adafruit_BME280 bme; // I2C
WiFiClient      client;
OTA             ota;

volatile bool published = false;
volatile long timestampPublished;

float temp     = 0.0;
float hum      = 0.0;
float pressure = 0.0;
float alt      = 0.0;

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
        while (1)
            ;
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

    if (false == published) {
        timestampPublished = millis();
        digitalWrite(LED_BUILTIN, HIGH);

        temp     = bme.readTemperature() + TEMPERATURE_OFFSET;
        hum      = bme.readHumidity();
        pressure = bme.readPressure();
        alt      = bme.readAltitude(SEALEVELPRESSURE_HPA);

        Blynk.virtualWrite(V1, temp);
        Blynk.virtualWrite(V2, hum);
        Blynk.virtualWrite(V3, pressure);
        Blynk.virtualWrite(V4, alt);

        Serial.println("> Reading sensor data and publishing to Blynk.");
        digitalWrite(LED_BUILTIN, LOW);

        published = true;
    }

    // If power saving features are enabled 
    if (true == CommonConfig::powerSaving) {
        // go to deep sleep once the WAKE_DURATION is reached.
        if (true == published && (millis() - timestampPublished >= WAKE_DURATION)) {
            Serial.println("> Going into a deep sleep. zzZZ");
            ESP.deepSleep(SLEEP_DURATION);
        }
    } else if (false == CommonConfig::powerSaving) {
        static uint16_t last_msg;

        // Once the publish interval is reached in NORMAL mode (non power-saving)
        // set published to false
        if (!timestampPublished || millis() - timestampPublished >= PUBLISH_INTERVAL) {
            published = false;
            timestampPublished = millis();
        }
    }
}