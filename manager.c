#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "err.h"
#include "common.h"

void create_pipe(int* cur_pipe) {
 	if (pipe(cur_pipe) == -1)
		syserr("Error in pipe()\n");
}

void close_pipe(int* cur_pipe) {
	if (close(cur_pipe[0]) == -1)
		syserr("Error in close_pipe(%d)\n", cur_pipe[0]);
	if (close(cur_pipe[1]) == -1)
		syserr("Error in close_pipe(%d)\n", cur_pipe[1]);
}

void prepare_input(int* cur_pipe) {
	if (close(0) == -1)
		syserr("Error in close(0)\n");
	if (dup(cur_pipe[0]) != 0)
		syserr("Error in dup(%s)\n", cur_pipe[0]);
}

void prepare_output(int* cur_pipe) {
	if (close(1) == -1)
		syserr("Error in close(1)\n");
	if (dup(cur_pipe[1]) != 1)
		syserr("Error in dup(%s)\n", cur_pipe[1]);
}

void make_loop(int n, int* last_pipe, char* grammar) {
    // last_pipe[0] = input of the recently created pipe
    // last_pipe[1] = output of the first pipe
	pid_t pid;
	if (n == 1) {
		prepare_input(last_pipe);
        prepare_output(last_pipe);
        close_pipe(last_pipe);
        return;
	}
	int next_pipe[2];
    create_pipe(next_pipe);
	switch (pid = fork()) {
		case -1:
			syserr("Error in fork()\n");
		case 0:
			prepare_input(last_pipe);
            close_pipe(last_pipe);
            prepare_output(next_pipe);
            close_pipe(next_pipe);
			execl("./worker", "./worker", grammar, NULL);
			syserr("Error in execl()\n");
		default:
            last_pipe[0] = next_pipe[0];
            make_loop(n - 1, last_pipe, grammar);
            return;
	}
}


int main(int argc, char* argv[]) {
	int n,i;
	int last_pipe[2];
    struct grammar g;

	if (argc != 4) {
		fatal("Usage: %s <workers number> <grammar file> <starting symbol>\n",
            argv[0]);
	}
	n = atoi(argv[1]);
	if (n <= 0) {
		fatal("Number of workers should be a positive integer");
	}
    if (strlen(argv[3]) != 1 || !is_upper(argv[3][0])) {
        fatal("Starting symbol should be a single nonterminal (uppercase)\n");
    }

    create_pipe(last_pipe);
	make_loop(n, last_pipe, argv[2]);

    read_grammar(&g, argv[2]);
    work(&g, argv[3][0]);

    for (i = 1; i < n; i++) {
		if (wait(0) == -1) syserr("Error in wait()\n");
	}
	return 0;
}

