#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include "sched.h"
#include "queue.h"
#include "err.h"

typedef struct thread_struct
{
    mysched_thread_t id;
    char* name;
    void (*starter)();
    jmp_buf jump;
} thread_data;

static Queue working;
static thread_data* current;
static jmp_buf main_loop_jump;

void timelimit_handler(int nr)
{
    printf("Timelimit_handler\n");
    switch (setjmp(current->jump)) {
        case -1:
            syserr("setjump\n");
        case 0:
            longjmp(main_loop_jump, 1);
            syserr("longjump\n");
        default:
            break;
    }
}
void sigprof() {
    /* preparing signal SIGPROF */
    struct sigaction mysignal, oldsignal;
    mysignal.sa_handler = timelimit_handler;
    sigemptyset(&mysignal.sa_mask);
    mysignal.sa_flags = 0;
    if (sigaction(SIGPROF, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");
}

void mysched_init()
{
	printf("Init()\n");

    sigprof();
    queue_init(&working);
}

void signal_handler(int nr)
{
    printf("Signal handler\n");
    switch(setjmp(current->jump)) {
        case -1:
            syserr("setjump\n");
            break;
        case 0:
            break;
        default:
            current->starter();
            printf("Po wyjsciu z funkcji\n");
            /* Clean up */
            free(current->name);
            current = NULL;
            longjmp(main_loop_jump, 1);
    }
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

    /* Preparing signal which will use this stack */
    mysignal.sa_handler = signal_handler;
    sigemptyset(&mysignal.sa_mask);
    mysignal.sa_flags = SA_ONSTACK;
    if (sigaction(SIGUSR1, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");

    /* Creating info about new thread */
    current = malloc(sizeof(thread_data));
    if (current == NULL)
        syserr("malloc\n");
    current->id = thread_counter;
    int name_len = strlen(name);
    current->name = (char*) malloc(name_len + 1);
    if (current->name == NULL)
        syserr("malloc\n");
    strcpy(current->name, name);
    current->starter = starter;
    raise(SIGUSR1);
    queue_push(working, (void*) current);
    current = NULL;

    thread_counter++;
	return thread_counter;
}

mysched_thread_t mysched_self() {
	return current->id;
}

char * mysched_get_name(mysched_thread_t thread) {
	return current->name;
}

//ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset);
//ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset);
void set_itimer()
{
    struct itimerval mytimer, oldtimer;
    getitimer(ITIMER_PROF, &mytimer);
    mytimer.it_value.tv_sec = 0;
    mytimer.it_value.tv_usec = 100000;
    mytimer.it_interval.tv_sec = 0;
    mytimer.it_interval.tv_usec = 100000;
    if (setitimer(ITIMER_PROF, &mytimer, &oldtimer) == -1)
        syserr("setitimer\n");
}

void mysched_go() {
	printf("Go()\n");
    if (setjmp(main_loop_jump) == -1)
        syserr("setjmp\n");
    if (current != NULL) queue_push(working, (void*) current);
    set_itimer();
    while (!queue_is_empty(working)) {
        current = (thread_data*) queue_front(working);
        queue_pop(working);
        printf("W glownej petli, funkcja id =%d\n", current->id);
        longjmp(current->jump, 1);
        syserr("longjump\n");
    }
    /* Clean up */
}

