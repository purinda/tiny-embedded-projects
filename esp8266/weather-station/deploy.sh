echo "Copying application to ESP8266..."
ampy --port /dev/tty.SLAB_USBtoUART put ../../config/global_config.py
ampy --port /dev/tty.SLAB_USBtoUART put boot.py
ampy --port /dev/tty.SLAB_USBtoUART put simple.py
ampy --port /dev/tty.SLAB_USBtoUART put robust.py
ampy --port /dev/tty.SLAB_USBtoUART put main.py
ampy --port /dev/tty.SLAB_USBtoUART put bme280.py
echo "Done copying."