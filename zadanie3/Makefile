CC = gcc
CFLAGS = -Wall
LFLAGS = -lm
OBJS = sched.o klient.o
EXEC = klient

all: $(EXEC)

klient: klient.o sched.o
	$(CC) $(LFLAGS) $+ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

klient.o: sched.h

clean:
	rm -f *.o 
	rm -f *~
	rm -f $(EXEC)

.PHONY: clean
