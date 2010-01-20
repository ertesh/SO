#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include <aio.h>
#include "sched.h"
#include "queue.h"
#include "err.h"

typedef struct thread_struct
{
    mysched_thread_t id;
    char* name;
    void (*starter)();
    void* mem;
    struct aiocb* aio;
    jmp_buf jump;
} thread_data;

static Queue working, waiting;
static thread_data* current;
static jmp_buf main_loop_jump;


/* Signal handling functions */

void sigprof_handler(int nr)
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

void finalize_current()
{
    free(current->name);
    free(current->aio);
    //free(current->mem);
    free(current);
    current = NULL;
}

void sigusr1_handler(int nr)
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

void sigusr2_handler(int nr)
{
    int ile = queue_size(waiting);
    while (ile) {
        thread_data* el = (thread_data*) queue_front(waiting);
        queue_pop(waiting);
        if (aio_error(el->aio) == 0) {
            queue_push(working, el);
            return;
        } else {
            queue_push(waiting, el);
            ile--;
        }
    }
    fatal("SIGUSR2 raised, but no AIO finished\n");
}

void prepare_signal(int signo, int flags, void(*signal_fun)(int))
{
    struct sigaction mysignal, oldsignal;
    mysignal.sa_handler = signal_fun;
    if (sigemptyset(&mysignal.sa_mask) == -1)
        syserr("sigemptyset\n");
    mysignal.sa_flags = flags;
    if (sigaction(signo, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");
}


/* Auxiliary functions */

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
    prepare_signal(SIGPROF, 0, sigprof_handler);
    prepare_signal(SIGUSR2, 0, sigusr2_handler);
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
    current = (thread_data*) malloc(sizeof(thread_data));
    if (current == NULL)
        syserr("malloc\n");
    current->id = thread_counter;
    current->name = copy_string(name);
    current->starter = starter;

    create_alternative_stack();
    prepare_signal(SIGUSR1, SA_ONSTACK, sigusr1_handler);
    raise(SIGUSR1);
    queue_push(working, (void*) current);
    current = NULL;

	return thread_counter++;
}

void prepare_aio(struct aiocb* aio, int d, void *buf, size_t nbytes, off_t offset)
{
    aio = (struct aiocb*) malloc(sizeof(aio));
    if (aio == NULL)
        syserr("malloc\n");
    aio->aio_fildes = d;
    aio->aio_buf = buf;
    aio->aio_nbytes = nbytes;
    aio->aio_offset = offset;
    aio->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aio->aio_sigevent.sigev_signo = SIGUSR2;
}

ssize_t common_aio(int (*fun)(struct aiocb*), int d, void *buf, size_t nbytes,
                   off_t offset)
{
	printf("common_aio\n");
   /* Common interface for functions mysched_pwrite and mysched_pread */
   prepare_aio(current->aio, d, buf, nbytes, offset);
   fun(current->aio);
   switch (setjmp(current->jump)) {
        case -1:
            syserr("setjump\n");
        case 0:
            queue_push(waiting, current);
            current = NULL;
            longjmp(main_loop_jump, 1);
            syserr("longjump\n");
        default:
            return aio_return(current->aio);
    }
}

ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset)
{
    return common_aio(aio_write, d, (void*) buf, nbytes, offset);
}

ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset)
{
    return common_aio(aio_read, d, buf, nbytes, offset);
}

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

