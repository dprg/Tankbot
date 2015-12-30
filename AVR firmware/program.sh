# avrdude -p m161 -c avr910 -P /dev/ttyUSB0 -e -U flash:w:"main.hex"
avrdude -p m16 -c avrisp2 -P usb -e -U flash:w:"main.hex"
