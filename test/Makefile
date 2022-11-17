compile: test.ino
	arduino-cli compile -b 'esp32:esp32:esp32' .

upload: compile
	arduino-cli upload -p /dev/ttyUSB0  --fqbn esp32:esp32:esp32 test

monitor: upload
	screen /dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0 115200
