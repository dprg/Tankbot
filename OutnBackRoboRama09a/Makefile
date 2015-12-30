CC = g++
CFLAGS = -g -Wall -Werror
CPPFLAGS = -g -Wall -Werror

DRIVE_OBJECTS = main.o tankbot.o odo.o pid.o MotorControl.o Logger.o \
	Configurator.o Navigator.o Compass.o Odometer.o Commander.o \
	Sensor.o
ES_OBJECTS = estop.o tankbot.o
SS_OBJECTS = sstop.o tankbot.o
RCOM_OBJECTS = rcom.o tankbot.o
MOVE_OBJECTS = move.o tankbot.o odo.o

BINARIES = drive estop tb_sim move sstop rcom

all:	$(BINARIES)

drive:	$(DRIVE_OBJECTS)
	$(CC) -o drive $(DRIVE_OBJECTS)

estop:	$(ES_OBJECTS)
	$(CC) -o estop $(ES_OBJECTS)

sstop:	$(SS_OBJECTS)
	$(CC) -o sstop $(SS_OBJECTS)

rcom:	$(RCOM_OBJECTS)
	$(CC) -o rcom $(RCOM_OBJECTS)

tb_sim:	tb_sim.c
	$(CC) $(CFLAGS) -o tb_sim tb_sim.c

move:	$(MOVE_OBJECTS)

odo.o:		odo.h odo.c
tankbot.o:	tankbot.h tankbot.c
pid.o:		pid.h pid.cpp
MotorControl.o:	MotorControl.h tankbot.h odo.h MotorControl.cpp
Logger.o:	Logger.h Logger.cpp
Configurator.o:	Configurator.h Configurator.cpp
Navigator.o:	Navigator.h Navigator.cpp
Compass.o:	Compass.h Sensor.h  Compass.cpp
Odometer.o:	Odometer.h Sensor.h Odometer.cpp
Commander.o:	Commander.h Commander.cpp
Sensor.o:	Sensor.h Sensor.cpp

clean: 
	rm -rf  *.o *.exe

