all: worker manager

manager: manager.o err.o
	gcc -Wall -o manager manager.o err.o

worker: worker.o err.o
	gcc -Wall -o worker worker.o err.o

err.o: err.c err.h
	gcc -Wall -c err.c

clean:
	rm -f *.o child_pipe parent_pipe parent_dup test_fifo

