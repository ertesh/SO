CC = gcc
CFLAGS = -Wall
LFLAGS = -lm
OBJS = sched.o queue.o err.o
LIBNAME = sched.a

all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	ar rs $@ $+

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

sched.o: err.h queue.h

queue.o: err.h

clean:
	rm -f *.o
	rm -f *~
	rm -f $(EXEC)

.PHONY: clean
