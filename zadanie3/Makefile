CC = gcc
CFLAGS = -Wall
LFLAGS = -lm
OBJS = klient.o sched.o queue.o err.o
EXEC = klient

all: $(EXEC)

klient: $(OBJS)
	$(CC) $(LFLAGS) $+ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

klient.o: sched.h

sched.o: err.h queue.h

clean:
	rm -f *.o 
	rm -f *~
	rm -f $(EXEC)

.PHONY: clean
