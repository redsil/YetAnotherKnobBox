#BOARD_TAG = uno (see make show_boards)
#ARDUINO_LIBS = <space separated list of libs, arduino.mk will try to guess>
MONITOR_PORT = /dev/ttyUSB0 (will be automatically guessed from IDE prefs)
include /usr/share/arduino/Arduino.mk
