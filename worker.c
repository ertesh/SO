#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>

int main(int argc, char* argv[]) {
	int ile;
	read(0, &ile, sizeof(int));
	write(2, &ile, sizeof(int));
	ile++;
	write(1, &ile, sizeof(int));
	wait(0);
	return 0;
}

