import paho.mqtt.publish as publish

host = "10.10.0.17"

msgs = [{'topic':"home/sensors/outdoor/temperature", 'payload':"30"},
    ("home/sensors/outdoor/humidity", "70", 0, False)]

publish.multiple(msgs, hostname=host)
