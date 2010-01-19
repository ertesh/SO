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
    void* mem;
    jmp_buf jump;
} thread_data;

static Queue working;
static thread_data* current;
static jmp_buf main_loop_jump;


/* Timer handling functions */

void timer_handler(int nr)
{
    switch (setjmp(current->jump)) {
        case -1:
            syserr("setjump\n");
        case 0:
            longjmp(main_loop_jump, 1);
            syserr("longjump\n");
        default:
            break;
            /* Do nothing, return to current->starter() */
    }
}

void prepare_sigprof() {
    struct sigaction mysignal, oldsignal;
    mysignal.sa_handler = timer_handler;
    if (sigemptyset(&mysignal.sa_mask) == -1)
        syserr("sigemptyset\n");
    mysignal.sa_flags = 0;
    if (sigaction(SIGPROF, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");
}

void set_timer()
{
    int limit = 100000;
    struct itimerval mytimer = {{0, 0}, {0, limit}};
    if (setitimer(ITIMER_PROF, &mytimer, NULL) == -1)
        syserr("setitimer\n");
}

void unblock_signal(int nr)
{
    sigset_t mysigset;
    if (sigemptyset(&mysigset) == -1)
        syserr("sigemptyset\n");
    if (sigaddset(&mysigset, SIGPROF) == -1)
        syserr("sigaddset\n");
    if (sigprocmask(SIG_UNBLOCK, &mysigset, NULL) == -1)
        syserr("sigprocmask\n");
}

/* Auxiliary functions */

void finalize_current()
{
    free(current->name);
    //free(current->mem);
    free(current);
    current = NULL;
}

void signal_handler(int nr)
{
    switch(setjmp(current->jump)) {
        case -1:
            syserr("setjump\n");
            break;
        case 0:
            break;
        default:
            /* Run given function */
            current->starter();
            finalize_current();
            longjmp(main_loop_jump, 1);
    }
}

void create_alternative_stack()
{
    stack_t mystack, defstack;
    void* mem = malloc(MYSCHED_STACK_SIZE);
    if (mem == NULL)
        syserr("malloc\n");
    current->mem = mem;
    mystack.ss_sp = mem;
    mystack.ss_flags = 0;
    mystack.ss_size = MYSCHED_STACK_SIZE;
    if (sigaltstack(&mystack, &defstack) == -1)
        syserr("sigaltstack\n");
}

void prepare_sigusr1()
{
    struct sigaction mysignal, oldsignal;
    mysignal.sa_handler = signal_handler;
    sigemptyset(&mysignal.sa_mask);
    mysignal.sa_flags = SA_ONSTACK;
    if (sigaction(SIGUSR1, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");
}

char* copy_string(const char* old_copy)
{
    int len = strlen(old_copy);
    char* new_copy = (char*) malloc(len + 1);
    if (new_copy == NULL)
        syserr("malloc\n");
    strcpy(new_copy, old_copy);
    return new_copy;
}

/* Main library functions */

void mysched_init()
{
    prepare_sigprof();
    queue_init(&working);
}

mysched_thread_t mysched_self() {
	return current->id;
}

char* mysched_get_name(mysched_thread_t thread) {
	return copy_string(current->name);
}

mysched_thread_t mysched_create_thread(void (*starter)(), const char *name) {
	static int thread_counter = 0;

    // All info about current thread will be stored in `current`
    current = malloc(sizeof(thread_data));
    if (current == NULL)
        syserr("malloc\n");
    current->id = thread_counter;
    current->name = copy_string(name);
    current->starter = starter;

    create_alternative_stack();
    prepare_sigusr1();
    raise(SIGUSR1);
    queue_push(working, (void*) current);
    current = NULL;

	return thread_counter++;
}

//ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset);
//ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset);

void mysched_go() {
    if (setjmp(main_loop_jump) == -1)
        syserr("setjmp\n");
    if (current != NULL) queue_push(working, (void*) current);
    while (!queue_is_empty(working)) {
        current = (thread_data*) queue_front(working);
        queue_pop(working);
        printf("W glownej petli, funkcja id =%d\n", current->id);
        unblock_signal(SIGPROF);
        set_timer();
        longjmp(current->jump, 1);
        syserr("longjump\n");
    }
    if (current != NULL)
        fatal("There should be no current thread\n");
    if (!queue_is_empty(working))
        fatal("Thread queue should be empty");
}

