PROG=live
SRC=main.c tankbot.c odo.c

all:	$(PROG)

$(PROG):	$(SRC)
	gcc -Wall -lm -o $(PROG) main.c tankbot.c odo.c

clean: 
	rm -rf  *.o $(PROG)
