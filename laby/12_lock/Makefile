CC          := gcc
CFLAGS      := -Wall -c
LFLAGS      := -Wall -o
ALL         := f_lock p_lock m_lock no_lock

all: $(ALL)

f_lock: lock.o flock-lib.o rw-lib.o err.o
	$(CC) $(LFLAGS) f_lock lock.o flock-lib.o rw-lib.o err.o

p_lock: lock.o lock-lib.o rw-lib.o err.o
	$(CC) $(LFLAGS) p_lock lock.o lock-lib.o rw-lib.o err.o

m_lock: m_lock.o lock-lib.o rw-lib.o err.o
	$(CC) $(LFLAGS) m_lock m_lock.o lock-lib.o rw-lib.o err.o

no_lock: no-lock.o rw-lib.o err.o
	$(CC) $(LFLAGS) no_lock no-lock.o rw-lib.o err.o

m_lock.o: lock.c lock-lib.h rw-lib.h err.h
	$(CC) -DMANDATORY $(CFLAGS) -o m_lock.o lock.c

lock.o: lock.c lock-lib.h rw-lib.h err.h
	$(CC) $(CFLAGS) lock.c

no-lock.o: no-lock.c rw-lib.h err.h
	$(CC) $(CFLAGS) no-lock.c

flock-lib.o: flock-lib.c lock-lib.h rw-lib.h err.h
	$(CC) $(CFLAGS) flock-lib.c

lock-lib.o: lock-lib.c lock-lib.h rw-lib.h err.h
	$(CC) $(CFLAGS) lock-lib.c

rw-lib.o: rw-lib.c rw-lib.h err.h
	$(CC) $(CFLAGS) rw-lib.c

err.o: err.c err.h
	$(CC) $(CFLAGS) err.c

clean:
	rm -f *.o $(ALL)

mrproper:
	rm -f *.o $(ALL) core *~
