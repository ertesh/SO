CC = gcc
CFLAGS = -Wall --pedantic -c
LDFLAGS = -Wall --pedantic

OBJECTS = err.o
ALL = signals cleanup ctrl-c child

all: $(ALL)

%.o : %.c %.h
	$(CC) $(CFLAGS) $<

$(ALL) : % : %.o $(OBJECTS)       
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -rf *.o $(ALL) *~
