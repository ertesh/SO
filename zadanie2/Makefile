CC = gcc
CFLAGS = -Wall
LFLAGS = -lm -pthread
OBJS = serwer.o klient.o err.o mesg.o graph.o
EXEC = serwer klient

all: $(EXEC)

serwer: serwer.o err.o mesg.o graph.o
	$(CC) $(LFLAGS) $+ -o $@

klient: klient.o err.o mesg.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

klient.o: err.h mesg.h
serwer.o: err.h mesg.h graph.h

clean:
	rm -f *.o 
	rm -f *~
	rm -f $(EXEC)

.PHONY: clean
