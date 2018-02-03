import robust
import machine
import utime as time
import gc
import bme280

NAME = "weather-station-1"
STATUS_LED_PIN = 2
MQTT_BROKER_ADDR = "10.10.0.17"
TOPIC = b"home/sensors/" + NAME

client = robust.MQTTClient(NAME, MQTT_BROKER_ADDR)
pin5 = machine.Pin(STATUS_LED_PIN, machine.Pin.OUT)

i2c = machine.I2C(scl=machine.Pin(16), sda=machine.Pin(5))
bme = bme280.BME280(i2c=i2c)

def initialise():
    blinkcnt = 0
    checkwifi()
    while blinkcnt < 50:
        pin5.value(blinkcnt % 2)
        blinkcnt = blinkcnt + 1
        time.sleep_ms(100)

    client.connect()

def checkwifi():
    blinkcnt = 0
    while not sta_if.isconnected():
        time.sleep_ms(500)
        pin5.value(blinkcnt % 2)
        blinkcnt = blinkcnt + 1

def publish():
    count = 1
    while True:
        pin5.value(0)
        checkwifi()
        v = bme.values

        temperature = v[0][:-1]
        pressure = v[1][:-3]
        humidity = v[2][:-1]

        client.publish(TOPIC, str(humidity))
        
        pin5.value(1)
        count = count + 1
        time.sleep(60)

# Initiate the program
initialise()
publish()
