CC = gcc

all: stnc

stnc: stnc.o handlers.o util.o
	$(CC) stnc.o handlers.o util.o -o stnc

stnc.o: stnc.c stnc.h
	$(CC) -c stnc.c -o stnc.o

handlers.o:  handlers.c	 handlers.h
	$(CC) -c handlers.c -o handlers.o

util.o: util.c util.h
	$(CC) -c util.c -o util.o	

clean:
	rm -f stnc  *.o *.txt