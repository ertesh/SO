CC=gcc
CFLAGS=-Wall -ansi -pedantic

all: serwer klient 

serwer: serwer.o err.o
	gcc -Wall -o serwer serwer.o err.o

klient: klient.o err.o
	gcc -Wall -o klient klient.o err.o

manager.o: manager.c common.h err.h
	gcc -Wall -c manager.c

worker.o: worker.c common.h err.h
	gcc -Wall -c worker.c

common.o: common.h common.c
	gcc -Wall -c common.c

err.o: err.c err.h
	gcc -Wall -c err.c

clean:
	rm -f *.o worker manager

