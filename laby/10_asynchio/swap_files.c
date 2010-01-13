#include<fcntl.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include"err.h"

#define BUF_SIZE (1024*1024)

/*
 * Zwraca rozmiar pliku spod podanego deskryptora
 */

int get_file_size(int fd)
{
    struct stat sb;
    if(fstat(fd, &sb) < 0)
	syserr("fstat");
    return sb.st_size;
}

/*
   Zamienia BUF_SIZE bajtow w podanych plikach,
   poczawszy od bajtu chunk_no*BUF_SIZE
*/

int swap_files(int fd1, int fd2, int size1, int size2, int chunk_no)
{

}


int main(int argc, char* argv[])
{
    int fd1, fd2, size1, size2, max_size, i;

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

    if(size1 > size2)
	max_size = size1;
    else
	max_size = size2;

    for(i=0; i<(max_size+BUF_SIZE-1)/BUF_SIZE; i++)
	swap_files(fd1, fd2, size1, size2, i);

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
