/**
 * Weather Station Application
 * Supports reading and reporting of 
 *  - Temperature
 *  - Humidity
 *  - Atmosphereic pressure
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
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_I2C_ADDR 0x76
Adafruit_BME280 bme;  // I2C

#define WLAN_SSID "Raspberry"
#define WLAN_PASS "Chat5w00d"

WiFiClient client;
PubSubClient pubsubClient;
long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;

const char TEMPERATURE_FEED[] PROGMEM = "home/sensors/outdoor/temperature";
const char PRESSURE_FEED[] PROGMEM = "home/sensors/outdoor/pressure";
const char HUMIDITY_FEED[] PROGMEM = "home/sensors/outdoor/humidity";
const char MQTT_SERVER[] PROGMEM = "10.10.0.17";
const int MQTT_PORT = 1883;

void reconnect() {
  // Loop until we're reconnected
  while (!pubsubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (pubsubClient.connect("HomeWeatherStation")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubsubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  digitalWrite(LED_BUILTIN, LOW);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(10);

  if (!bme.begin(BME280_I2C_ADDR)) {
    Serial.print(
        "Ooops, no BME280 detected ... Check your wiring or I2C ADDR!");
    while (1)
      ;

  } else {
    Serial.println("BME280 ready.");
  }

  // Connect to WiFi access point.
  Serial.println();
  Serial.print("Wireless MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

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

  pubsubClient.setClient(client);
  pubsubClient.setServer(MQTT_SERVER, MQTT_PORT);
  delay(10000);
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void loop() {
  if (!pubsubClient.connected()) {
    reconnect();
  }
  pubsubClient.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    float newTemp = bme.readTemperature();
    float newHum = bme.readHumidity();

    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      pubsubClient.publish(TEMPERATURE_FEED, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      pubsubClient.publish(HUMIDITY_FEED, String(hum).c_str(), true);
    }
  }
}
