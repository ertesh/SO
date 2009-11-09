all: worker manager

manager: manager.o common.o err.o
	gcc -Wall -o manager manager.o common.o err.o

worker: worker.o common.o err.o
	gcc -Wall -o worker worker.o common.o err.o

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

