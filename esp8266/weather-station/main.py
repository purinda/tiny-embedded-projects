"""
    Weather Station Application (works with Home Assistant)

    Purpose of this project is to use BME280 weather sensor to collect data and
    send that information to MQTT server. Then applications like "Home Assistant" 
    can use that information.
 
    Supports reading and reporting of 
    - Temperature
    - Humidity
    - Atmosphereic pressure
"""
import robust
import machine
import utime as time
import gc
import bme280
from Parameters import MQTTServerConfig

NAME = "weather-station-1"
STATUS_LED_PIN = 2
TOPIC = b"home/sensors/" + NAME

client = robust.MQTTClient(NAME, MQTTServerConfig.BROKER_IP)
status_led = machine.Pin(STATUS_LED_PIN, machine.Pin.OUT)

i2c = machine.I2C(scl=machine.Pin(16), sda=machine.Pin(5))
bme = bme280.BME280(i2c=i2c)

def initialise():
    blinkcnt = 0
    checkwifi()
    while blinkcnt < 50:
        status_led.value(blinkcnt % 2)
        blinkcnt = blinkcnt + 1
        time.sleep_ms(100)

    client.connect()

def checkwifi():
    blinkcnt = 0
    while not sta_if.isconnected():
        time.sleep_ms(500)
        status_led.value(blinkcnt % 2)
        blinkcnt = blinkcnt + 1

def publish():
    count = 1
    while True:
        status_led.value(0)
        checkwifi()
        v = bme.values

        msg = b'{"MsgId":%u,"Mem":%u,"Temperature":%s,"Pressure":%s,"Humidity":%s}' 
            % (count, gc.mem_free(), v[0][:-1], v[1][:-3], v[2][:-1])
        client.publish(TOPIC, msg)
        
        status_led.value(1)
        count = count + 1
        time.sleep(30)

# Initiate the program
initialise()
publish()
