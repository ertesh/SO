/* 
	aio-copy-readahead.c - Creating multiple copies of file in one thread using 
	asynchronous I/O with readahead.
	
	Description:
	NUM_ASYNC_IO parts are read asynchronously from source file.
	For each read part asynchronous write operation to all copy 
	files is initiated. When all copies are written, next part 
	of source file is read.
	Additionally, NUM_READAHEAD_BUFFERS of additional reads are 
	performed as readahead. If such read is ready, it is taken instead
	of performing normal read.
	
	Author: Krzysztof Lichota <lichota@mimuw.edu.pl>
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include <aio.h>

//define to show status of completing requests
#define SHOW_IO_STATUS

enum
{
	//number of concurrent IO/s per file copied
	NUM_ASYNC_IO = 5,
	BUFFER_SIZE = 512,
	NUM_READAHEAD_BUFFERS = 10,
};

//information for each copy file
struct copy_info
{
	//file descriptor
	int fd;
	
	struct aiocb aiocb_blocks[NUM_ASYNC_IO];
};

//number of copies
int num_copies; 

//array of info for each copy
struct copy_info *copies;

//number of pointers to aiocb structures in aiocb_ptrs array
int num_aiocb_ptrs;
	
//array of pointers to asynchronous i/o blocks
struct aiocb **aiocb_ptrs;

//descriptor of source file
int source_file_fd;

//size of source file
int source_file_size;

//how many bytes were already read from source file
int source_file_read;
	
//buffer for NUM_ASYNC_IO read blocks of BUFFER_SIZE size
char buffers[BUFFER_SIZE*NUM_ASYNC_IO];
	
//number of writes left for each slot
int writes_left[NUM_ASYNC_IO];
	
//total number of outstanding writes
int total_writes_left;
	
//total number of outstanding reads
int total_reads_left;

//aiocb blocks
struct read_info
{
	struct aiocb read_aiocb;
	//if true data was read ahead instead of direct read
	uint readahead;
	//buffer which keeps data, if readahead is true, it is buffer allocated in readahead
	char *buffer;
};

struct read_info read_aiocb_blocks[NUM_ASYNC_IO];

//flag set when finished copying
int finished;

//number of readahead reads started
uint num_readahead_started = 0;

//number of readahead buffers which have data ready
uint num_readahead_ready = 0;

struct readahead_info
{
	struct aiocb readahead_aiocb;
	//if true data has been read
	uint ready;
};

struct readahead_info readahead_aiocb_blocks[NUM_READAHEAD_BUFFERS];

//forward declarations
void start_readahead(int readaheadslot);
void start_writes(uint slot);

inline struct aiocb **write_aiocb_ptr(int copy, int slot)
{
	//first NUM_ASYNC_IO entries are for read aiocbs
	//next NUM_READAHEAD_BUFFERS for readahead
	return &aiocb_ptrs[(copy+1)*NUM_ASYNC_IO+NUM_READAHEAD_BUFFERS+slot];
}

inline struct aiocb **read_aiocb_ptr(int slot)
{
	return &aiocb_ptrs[slot];
}

inline struct aiocb **readahead_aiocb_ptr(int readaheadslot)
{
	return &aiocb_ptrs[NUM_ASYNC_IO+readaheadslot];
}

void do_start_read(int slot);

uint start_async_read(
	struct aiocb *aio,
	struct aiocb **aiocb_ptr,
	char *buffer
	)
{
	int err;
	int left;
	
	left = source_file_size - source_file_read;
	
	if (left == 0)
	{
		//nothing left to read
		//check if we have finished everything
		return 0;
	}
	
	total_reads_left++;
	
	if (left > BUFFER_SIZE)
	{
		left = BUFFER_SIZE;
	}

	aio->aio_fildes = source_file_fd;
	
	aio->aio_offset = source_file_read;
	aio->aio_buf = buffer;
	aio->aio_nbytes	= left;
	
	//no notification
	aio->aio_sigevent.sigev_notify = SIGEV_NONE;
	
	source_file_read += left;
	
	err = aio_read(aio);
	
	if (err == -1)
	{
		perror("aio_read failed");
		exit(1);
	}

	//set pointer in aiocb_ptrs table
	*aiocb_ptr = aio;
	return 1;
}

void do_start_read(int slot)
{
	struct aiocb *aio = &(read_aiocb_blocks[slot].read_aiocb);
	struct aiocb **aiocb_ptr = read_aiocb_ptr(slot);
	char * buffer = &buffers[slot*BUFFER_SIZE];
	
	read_aiocb_blocks[slot].readahead = 0;
	read_aiocb_blocks[slot].buffer = buffer;
	
	start_async_read(aio, aiocb_ptr, buffer);
}

void start_read(int slot)
{
	uint i;
	
	if (num_readahead_ready == 0)
	{
		//no readahead ready
		//start normal read
		do_start_read(slot);
		return;
	}
	
	//find ready readahead
	for (i = 0; i < NUM_READAHEAD_BUFFERS; ++i)
	{
		if (readahead_aiocb_blocks[i].ready)
		{
			//consume this readahead
			read_aiocb_blocks[slot].readahead = 1;
			
			//take over buffer from readahead
			read_aiocb_blocks[slot].buffer = (char *)readahead_aiocb_blocks[i].readahead_aiocb.aio_buf;
			read_aiocb_blocks[slot].read_aiocb.aio_offset = readahead_aiocb_blocks[i].readahead_aiocb.aio_offset;
			read_aiocb_blocks[slot].read_aiocb.aio_nbytes = readahead_aiocb_blocks[i].readahead_aiocb.aio_nbytes;
			
			num_readahead_ready--;
			
			start_writes(slot);
			
			start_readahead(i);
			
			return;
		}
	}
	//should never get here
	assert(0);
}

void start_readahead(int readaheadslot)
{
	struct aiocb *aio = &(readahead_aiocb_blocks[readaheadslot].readahead_aiocb);
	struct aiocb **aiocb_ptr = readahead_aiocb_ptr(readaheadslot);
	char *buffer = malloc(BUFFER_SIZE);
	
	if (buffer == NULL)
	{
		perror("malloc() failed");
		exit(1);
	}
	
	readahead_aiocb_blocks[readaheadslot].ready = 0;
	
	num_readahead_started++;
	
	start_async_read(aio, aiocb_ptr, buffer);
}

void start_write(int copy, int slot)
{
	int err;
	struct aiocb *aio = &(copies[copy].aiocb_blocks[slot]);

	aio->aio_fildes = copies[copy].fd;
	
	//offset, buffer and length are the same as for read
	aio->aio_offset = read_aiocb_blocks[slot].read_aiocb.aio_offset;
	aio->aio_buf = read_aiocb_blocks[slot].buffer;
	aio->aio_nbytes	= read_aiocb_blocks[slot].read_aiocb.aio_nbytes;
	
	//no notification
	aio->aio_sigevent.sigev_notify = SIGEV_NONE;
	
	err = aio_write(aio);
	
	if (err == -1)
	{
		perror("aio_write failed");
		exit(1);
	}

	//set pointer in aiocb_ptrs table
	*write_aiocb_ptr(copy, slot) = aio;
}

void check_completed_read(int slot)
{
	struct aiocb *aio;
	int status;
	
	if (read_aiocb_blocks[slot].readahead)
	{
		return;
	}
	
	aio = *read_aiocb_ptr(slot);
	
	if (aio == NULL)
	{
		return;
	}
	
	status = aio_error(aio);
	
	if (status == EINPROGRESS)
	{
		//request still in progress
#ifdef SHOW_IO_STATUS		
		fprintf(stderr, "r");
#endif
		return;
	}
#ifdef SHOW_IO_STATUS		
	fprintf(stderr, "R");
#endif
	
	if (status != 0)
	{
		fprintf(stderr, "Error in async read I/O processing (%d): %s\n", status, strerror(status));
		exit(1);
	}
	
	//request completed
	
	total_reads_left--;
	
	//clear entry in aiocb_ptrs table
	*read_aiocb_ptr(slot) = NULL;
	
	start_writes(slot);
}

void check_completed_readahead(int readaheadslot)
{
	struct aiocb *aio;
	int status;
	
	if (readahead_aiocb_blocks[readaheadslot].ready)
	{
		return;
	}
	
	aio = *readahead_aiocb_ptr(readaheadslot);
	
	if (aio == NULL)
	{
		return;
	}
	
	status = aio_error(aio);
	
	if (status == EINPROGRESS)
	{
		//request still in progress
#ifdef SHOW_IO_STATUS		
		fprintf(stderr, "h");
#endif
		return;
	}
#ifdef SHOW_IO_STATUS		
	fprintf(stderr, "H");
#endif
	
	if (status != 0)
	{
		fprintf(stderr, "Error in async read I/O processing (%d): %s\n", status, strerror(status));
		exit(1);
	}
	
	//request completed
	
	total_reads_left--;
	
	//clear entry in aiocb_ptrs table
	*readahead_aiocb_ptr(readaheadslot) = NULL;
	readahead_aiocb_blocks[readaheadslot].ready = 1;
	num_readahead_started--;
	num_readahead_ready++;
}

void start_writes(uint slot)
{
	uint copy;
	//start writing copies
	
	writes_left[slot] = num_copies;
	total_writes_left += num_copies;
	
	for (copy = 0; copy < num_copies; copy++)
	{
		start_write(copy, slot);
	}
	
}

void check_completed_write(int copy, int slot)
{
	struct aiocb *aio;
	int status;
	
	aio = *write_aiocb_ptr(copy, slot);
	
	if (aio == NULL)
	{
		return;
	}
	
	status = aio_error(aio);
	
	if (status == EINPROGRESS)
	{
		//request still in progress
#ifdef SHOW_IO_STATUS		
		fprintf(stderr, "w");
#endif
		return;
	}
	
#ifdef SHOW_IO_STATUS		
	fprintf(stderr, "W");
#endif
	
	if (status != 0)
	{
		fprintf(stderr, "Error in async write I/O processing (%d): %s\n", status, strerror(status));
		exit(1);
	}
	
	//request completed

	*write_aiocb_ptr(copy, slot) = NULL;
	writes_left[slot]--;
	total_writes_left--;
	
	if (writes_left[slot] == 0)
	{
		//all writes completed for this slot
		//try to start next read
		
		if (read_aiocb_blocks[slot].readahead)
		{
			//in readahead we malloced buffer
			free(read_aiocb_blocks[slot].buffer);
		}
		
		start_read(slot);
	}
	//check end condition
	if (total_reads_left == 0 && total_writes_left == 0 && num_readahead_ready == 0)
	{
		finished = 1;
	}
}

void copy_files()
{
	int i;
	
	//Start reading
	for(i = 0; i < NUM_ASYNC_IO; i++)
	{
		start_read(i);
	}

	for(i = 0; i < NUM_READAHEAD_BUFFERS; i++)
	{
		start_readahead(i);
	}

	do	
	{
		int copy;
		int slot;
		
		//check end condition
		if (total_reads_left == 0 && total_writes_left == 0 && num_readahead_ready == 0)
		{
			finished = 1;
			break;
		}
		
		if (aio_suspend((const struct aiocb *const *) aiocb_ptrs, num_aiocb_ptrs, NULL) == -1)
		{
			perror("aio_suspend");
			exit(1);
		}

		//check if readahead was completed		
		for (slot = 0; slot < NUM_READAHEAD_BUFFERS; slot++)
		{
			check_completed_readahead(slot);
		}
		
		//check if read was completed		
		for (slot = 0; slot < NUM_ASYNC_IO; slot++)
		{
			check_completed_read(slot);
		}
		
		//check if write was completed
		for (copy = 0; copy < num_copies; copy++)
		{		
			for (slot = 0; slot < NUM_ASYNC_IO; slot++)
			{
				check_completed_write(copy, slot);
			}
		}
#ifdef SHOW_IO_STATUS		
		fprintf(stderr, "\n");
#endif
	} while (!finished);
}

void init_global()
{
	int i;
	
	//alloc structures for copies
	copies = malloc(num_copies * sizeof(struct copy_info));
	
	if (copies == NULL)
	{
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	memset(copies, 0, num_copies * sizeof(struct copy_info));
	
	//num_copies for each copy
	//+1 for source file aiocbs
	//+NUM_READAHEAD_BUFFERS for readahead buffers
	num_aiocb_ptrs = (num_copies+1)*NUM_ASYNC_IO+NUM_READAHEAD_BUFFERS;
	
	aiocb_ptrs = malloc(num_aiocb_ptrs * sizeof(struct aiocb*));
	
	if (aiocb_ptrs == NULL)
	{
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	memset(aiocb_ptrs, 0, num_aiocb_ptrs * sizeof(struct aiocb*));
	
	for (i=0; i < NUM_ASYNC_IO; i++)
	{
		writes_left[i] = 0;
	}
	
	total_writes_left = 0;
	total_reads_left = 0;
	finished = 0;
}

void done_global()
{
	free(copies);
	free(aiocb_ptrs);
}

void init_source_file(char *source_file_name)
{
	int err;
	struct stat source_file_stat;
	uint i;
	
	//open source file
	source_file_fd = open(source_file_name, O_RDONLY);
	
	if (source_file_fd == -1) 
	{
		perror("open source file");
		exit(1);
	}
	
	//get size of source file
	err = fstat(source_file_fd, &source_file_stat);
	
	if (err == -1)
	{
		perror("Cannot stat source file");
		exit(1);
	}	
	
	source_file_size = source_file_stat.st_size;
	
	source_file_read = 0;
	
	for (i = 0; i < NUM_ASYNC_IO; ++i)
	{
		memset(&read_aiocb_blocks[i].read_aiocb, 0, sizeof(struct aiocb));
		read_aiocb_blocks[i].readahead = 0;
	}
	
	for (i = 0; i < NUM_READAHEAD_BUFFERS; ++i)
	{
		memset(&readahead_aiocb_blocks[i].readahead_aiocb, 0, sizeof(struct aiocb));
		readahead_aiocb_blocks[i].ready = 0;
	}
}

void done_source_file()
{
	close(source_file_fd);
}

void init_copy_files(char *source_file_name)
{
	char *copy_file_name;
	int i;
	int name_buf_len;
	
	//assume 2^32 possible copies :)
	//add 1 char for '.' and one for trailing NULL
	name_buf_len = strlen(source_file_name) + 10 + 1 + 1;
	copy_file_name = malloc(name_buf_len);
	
	if (copy_file_name == NULL)
	{
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	
	for (i=0; i < num_copies; i++)
	{
		int name_len = snprintf(copy_file_name, name_buf_len, "%s.%d", source_file_name, i);
		if (name_len > name_buf_len-1)
		{
			//name_buf_len-1 as trailing NULL must fit into buffer
			//name_len is without trailing NULL
			fprintf(stderr,"Copy file name does not fit in buffer");
			exit(1);
		}
		
		copies[i].fd = creat(copy_file_name, 0644);
		
		if (copies[i].fd == -1)
		{
			perror("Cannot open copy file");
			exit(1);
		}
	}
	
	free(copy_file_name);
}

void done_copy_files()
{
	int i;
	
	for (i=0; i < num_copies; i++)
	{
		close(copies[i].fd);
	}
}

int main(int argc, char ** argv) 
{
	char *source_file_name;
	
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s number_of_copies source_file_name\n", argv[0]);
		exit(1);
	}

	num_copies = atoi(argv[1]);
	source_file_name = argv[2];

	if (num_copies <= 0)
	{
		fprintf(stderr, "Number of copies must be > 0\n");
		exit(1);
	}
	
	init_global();	
	init_source_file(source_file_name);
	init_copy_files(source_file_name);
	
	copy_files();
	
	done_copy_files();
	done_source_file();
	done_global();

	return 0;
}
