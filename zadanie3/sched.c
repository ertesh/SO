#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sched.h"
#include "queue.h"
#include "err.h"

Queue working;
void mysched_init() {
	printf("Init()\n");

    queue_init(&working);
}

void signal_handler(int nr)
{
    printf("Signal handler\n");
}


mysched_thread_t mysched_create_thread(void (*starter)(), const char *name) {
	static int thread_counter = 0;
    stack_t mystack, defstack;
    struct sigaction mysignal, oldsignal;

    printf("Create()\n");

    /* Creating alternative stack */
    void* mem = malloc(MYSCHED_STACK_SIZE);
    if (mem == NULL)
        syserr("malloc\n");
    mystack.ss_sp = mem;
    mystack.ss_flags = 0;
    mystack.ss_size = MYSCHED_STACK_SIZE;
    if (sigaltstack(&mystack, &defstack) == -1)
        syserr("sigaltstack\n");

    /* Creating signal which will use this stack */
    mysignal.sa_handler = signal_handler;
    sigemptyset(&mysignal.sa_mask);
    mysignal.sa_flags = SA_ONSTACK;
    if (sigaction(SIGUSR2, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");

    raise(SIGUSR2);

    /* Creating info about new thread */
    thread_counter++;

	return thread_counter;
}

mysched_thread_t mysched_self() {
	return 0;
}

char * mysched_get_name(mysched_thread_t thread) {
	return 0;
}

//ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset);
//ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset);

void mysched_go() {
	printf("Go()\n");
    while (!queue_is_empty(working)) {
        /* DO IT */
    }
}

