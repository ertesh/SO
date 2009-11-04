#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "err.h"

void close_pipe(int* pipe) {
	if (close(pipe[0]) == -1)
		syserr("Error in close_pipe(%d)\n", pipe[0]);
	if (close(pipe[1]) == -1)
		syserr("Error in close_pipe(%d)\n", pipe[1]);		
}

void prepare_input(int* pipe) {
	if (close(0) == -1)
		syserr("Error in input.close(0)\n");
	if (dup(pipe[0]) != 0)
		syserr("Error in input.dup(pipe[0])\n");
	close_pipe(pipe);
}

void prepare_output(int* pipe) {
	if (close(1) == -1)
		syserr("Error in output.close(0)\n");
	if (dup(pipe[1]) != 1)
		syserr("Error in output.dup(pipe[1])\n");
	close_pipe(pipe);
}

void make_loop(int counter, int n, int* last_pipe, char* grammar) {
	pid_t pid;
	int pipe_dsc[2];
	if (pipe(pipe_dsc) == -1) 
		syserr("Error in pipe()\n");
	
	if (counter == 1)
		if (pipe(last_pipe) == -1) 
			syserr("Error in pipe()\n");
	
	if (counter == n) {
		prepare_output(last_pipe);
		execl("./worker", grammar, NULL);
		syserr("Error in execl()\n");
	}

	switch (pid = fork()) {
		case -1:
			syserr("Error in fork()\n");
		case 0:
			printf("potomny, pid=%d, fork=%d\n", getpid(), pid);
			prepare_input(pipe_dsc);
			make_loop(counter + 1, n, last_pipe, grammar);
			fatal("Error in make_loop %d\n",counter+1);

		default:
			printf("macierzysty, pid=%d, fork=%d\n", getpid(), pid);
			prepare_output(pipe_dsc);
			if (counter > 1) {
				close_pipe(last_pipe);
				execl("./worker", grammar, NULL);
				syserr("Error in execl()\n");
			} else {
				prepare_input(last_pipe);
				return;
			}	
	}
}

void work() {
	int ile = '0';
	write(1, &ile, sizeof(int));
	read(0, &ile, sizeof(int));
	write(2, &ile, sizeof(int));
}

int main(int argc, char* argv[]) {
	int n;
	int last_pipe[2];
	if (argc != 4) {
		fatal("Usage: %s <number of workers> <grammar file> <starting symbol>\n");
	}
	n = atoi(argv[1]);
	if (n <= 0) {
		fatal("Number of workers should be a positive integer");
	}
	make_loop(1, n, last_pipe, argv[2]);
	work();
//	for (i=1; i<n; i++) {
//		if (wait(0) == -1) syserr("Error in wait()\n");
//	}
	return 0;
}

