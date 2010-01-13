CFLAGS=-Wall -g -O0
CC=gcc
LIBS=-lrt

all: aio-copy swap_files_sync swap_files

aio-copy: aio-copy.c
	$(CC) $(CFLAGS) -o aio-copy aio-copy.c -lrt

aio-copy-readahead: aio-copy-readahead.c
	$(CC) $(CFLAGS) -o aio-copy-readahead aio-copy-readahead.c -lrt

swap_files: swap_files.o err.o
	$(CC) $(CFLAGS) -o swap_files swap_files.o err.o

swap_files_sync: swap_files_sync.o err.o
	$(CC) $(CFLAGS) -o swap_files_sync swap_files_sync.o err.o

clean:
	@rm *.o -f aio-copy aio-copy-readahead swap_files swap_files_sync
