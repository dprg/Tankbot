MCU=atmega16
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-g -mmcu=$(MCU) -Wall -Wstrict-prototypes -Os -mcall-prologues

all: main.hex

main.hex : main.out
	$(OBJCOPY) -R .eeprom -O ihex main.out main.hex

main.out : main.o commands.o util.o serial.o encoders.o pwm.o
	$(CC) $(CFLAGS) -o main.out -Wl,-Map,main.map main.o commands.o util.o serial.o encoders.o pwm.o

main.o : main.c
	$(CC) $(CFLAGS) -Os -c main.c

commands.o : commands.c
	$(CC) $(CFLAGS) -Os -c commands.c

util.o : util.c
	$(CC) $(CFLAGS) -Os -c util.c

serial.o : serial.c
	$(CC) $(CFLAGS) -Os -c serial.c

encoders.o : encoders.c
	$(CC) $(CFLAGS) -Os -c encoders.c 

pwm.o : pwm.c
	$(CC) $(CFLAGS) -Os -c pwm.c 


clean:
	rm -f *.o *.map *.out *t.hex
