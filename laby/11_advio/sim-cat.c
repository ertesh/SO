/* sim-cat.c -- simple version of cat */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** argv) {
    int fd, len;
    char buf[1024];

    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        perror("open");
        return 1;
    }
    
    /* len will be >= 0 while data is available, and read() is
       successful */
    while ((len = read(fd, buf, sizeof(buf))) > 0) {
        if (write(1, buf, len) != len) {
            perror("write");
            return 1;
        }
    }

    /* len was <= 0; If len = 0, no more data is available. 
       Otherwise, an error occurred. */
    if (len < 0) {
        perror("read");
        return 1;
    }

    return 0;
}
