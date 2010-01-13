#include<fcntl.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include"err.h"

#define BUF_SIZE (1024*1024)

char buf1[BUF_SIZE], buf2[BUF_SIZE];

int get_file_size(int fd)
{
    struct stat sb;
    if(fstat(fd, &sb) < 0)
	syserr("fstat");
    return sb.st_size;
}

int my_read(int fd, char* buf, size_t count)
{
    int offset, ret;
    offset = 0;
    while(count)
    {
	ret = read(fd, buf+offset, count);
	if(ret < 0)
	    syserr("read");

	if(ret == 0 && offset == 0)
	    return 0;

	if(ret == 0 && offset > 0)
	    return offset;
	count -= ret;
	offset += ret;
    }
    return offset;
}

void my_write(int fd, char* buf, size_t count)
{
    int offset, ret;
    offset = 0;
    while(count)
    {
	ret = write(fd, buf+offset, count);
	if(ret < 0)
	    syserr("read");
	count -= ret;
	offset += ret;
    }
}

int swap_files(int fd1, int fd2, int chunk_no)
{
    int r1, r2;
    if(lseek(fd1, chunk_no*BUF_SIZE, SEEK_SET) == (off_t)-1)
	syserr("lseek");
    if(lseek(fd2, chunk_no*BUF_SIZE, SEEK_SET) == (off_t)-1)
	syserr("lseek");

    r1 = my_read(fd1, buf1, BUF_SIZE);
    r2 = my_read(fd2, buf2, BUF_SIZE);
    
    if(r1 == 0 && r2 == 0)
	return 0;
    
    if(lseek(fd1, chunk_no*BUF_SIZE, SEEK_SET) == (off_t)-1)
	syserr("lseek");
    
    if(lseek(fd2, chunk_no*BUF_SIZE, SEEK_SET) == (off_t)-1)
	syserr("lseek");

    my_write(fd2, buf1, r1);
    my_write(fd1, buf2, r2);

    return 1;
}

int main(int argc, char* argv[])
{
    int fd1, fd2, size1, size2, i;

    if(argc < 3)
	fatal("Program przyjmuje 2 argumenty");
    
    fd1 = open(argv[1], O_RDWR);
    if(fd1 < 0)
	syserr("open");

    fd2 = open(argv[2], O_RDWR);
    if(fd2 < 0)
	syserr("open");

    size1 = get_file_size(fd1);
    size2 = get_file_size(fd2);

    for(i=0; ; i++)
	if(swap_files(fd1, fd2, i) == 0)
	    break;

    if(ftruncate(fd1, size2) < 0) /* "Przycina" plik do pozadanego rozmiaru */
	syserr("truncate");

    if(ftruncate(fd2, size1) < 0)
        syserr("truncate");

    if(close(fd1) < 0)
	syserr("close");
    
    if(close(fd2) < 0)
	syserr("close");

    return 0;
}
