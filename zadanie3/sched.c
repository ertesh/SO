#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include <aio.h>
#include <errno.h>
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

static void sigprof_handler(int nr)
{
    switch (setjmp(current->jump)) {
        case -1:
            syserr("setjump\n");
        case 0:
            queue_push(working, (void*) current);
            current = NULL;
            longjmp(main_loop_jump, 1);
            syserr("longjump\n");
        default:
            break;
            /* Do nothing, return to current->starter() */
    }
}

static void finalize_current()
{
    free(current->name);
    //free(current->mem);
    free(current);
    current = NULL;
}

static void sigusr1_handler(int nr)
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

static void sigusr2_handler(int nr)
{
    int ile = queue_size(waiting);
    while (ile) {
        thread_data* el = (thread_data*) queue_front(waiting);
        queue_pop(waiting);
        switch (aio_error(el->aio)) {
            case 0:
                queue_push(working, el);
                return;
            case EINPROGRESS:
                queue_push(waiting, el);
                ile--;
                break;
            default:
                syserr("aio_error\n");
        }
    }
    fatal("SIGUSR2 raised, but no AIO finished\n");
}

static void prepare_signal(int signo, int flags, void(*signal_fun)(int))
{
    struct sigaction mysignal, oldsignal;
    mysignal.sa_handler = signal_fun;
    if (sigemptyset(&mysignal.sa_mask) == -1)
        syserr("sigemptyset\n");
    mysignal.sa_flags = flags;
    if (sigaction(signo, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");
}

static void unblock_signal(int nr)
{
    sigset_t mysigset;
    if (sigemptyset(&mysigset) == -1)
        syserr("sigemptyset\n");
    if (sigaddset(&mysigset, SIGPROF) == -1)
        syserr("sigaddset\n");
    if (sigprocmask(SIG_UNBLOCK, &mysigset, NULL) == -1)
        syserr("sigprocmask\n");
}

/* AIO functions */

static void prepare_aio(int d, void *buf, size_t nbytes, off_t offset)
{
    struct aiocb* aio = (struct aiocb*) malloc(sizeof(struct aiocb));
    if (aio == NULL)
        syserr("malloc\n");
    aio->aio_fildes = d;
    aio->aio_buf = buf;
    aio->aio_nbytes = nbytes;
    aio->aio_offset = offset;
    aio->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aio->aio_sigevent.sigev_signo = SIGUSR2;
    current->aio = aio;
}

static void set_timer(int limit)
{
    struct itimerval mytimer = {{0, 0}, {0, limit}};
    if (setitimer(ITIMER_PROF, &mytimer, NULL) == -1)
        syserr("setitimer\n");
}

static ssize_t common_aio()
{
    /* Common interface for functions mysched_pwrite and mysched_pread */
    struct aiocb* aio;
    set_timer(0);
    switch (setjmp(current->jump)) {
        case -1:
            syserr("setjump\n");
        case 0:
            queue_push(waiting, current);
            current = NULL;
            longjmp(main_loop_jump, 1);
            syserr("longjump\n");
        default:
            aio = current->aio;
            free(current->aio);
            return aio_return(aio);
    }
}

/* Auxiliary functions */

static void create_alternative_stack()
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

static void suspend() {
    int list_len = queue_size(waiting);
    int list_size = list_len * sizeof(struct aiocb*);
    struct aiocb** list = (struct aiocb**) malloc(list_size);
    int k = list_len;
    while (k--) {
        thread_data* el = (thread_data*) queue_front(waiting);
        list[k] = el->aio;
        queue_pop(waiting);
        queue_push(waiting, el);
    }
    if (aio_suspend((const struct aiocb* const*) list, list_len, NULL) == -1)
        syserr("aio_suspend\n");
    free(list);
}

static char* copy_string(const char* old_copy)
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
    queue_init(&waiting);
}

mysched_thread_t mysched_self()
{
	return current->id;
}

char* mysched_get_name(mysched_thread_t thread)
{
	return copy_string(current->name);
}

mysched_thread_t mysched_create_thread(void (*starter)(), const char *name)
{
	static int thread_counter = 0;
    /* All info about current thread will be stored in `current` */
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

ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset)
{
    prepare_aio(d, (void*) buf, nbytes, offset);
    if (aio_write(current->aio) == -1)
       syserr("aio_write\n");
    return common_aio();
}

ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset)
{
    prepare_aio(d, buf, nbytes, offset);
    if (aio_read(current->aio) == -1)
       syserr("aio_read\n");
    return common_aio();
}

void mysched_go()
{
    if (setjmp(main_loop_jump) == -1)
        syserr("setjmp\n");
    /* Main loop */
    while (!queue_is_empty(working)) {
        current = (thread_data*) queue_front(working);
    // fprintf(stderr, "Funkcja numer %d (%s)\n", current->id, current->name);
        queue_pop(working);
        unblock_signal(SIGPROF);
        int limit = 100000; /* Time in microseconds given for thread */
        set_timer(limit);
        longjmp(current->jump, 1);
        syserr("longjump\n");
    }
    /* When all threads are waiting for IO */
    if (!queue_is_empty(waiting)) {
        suspend();
        longjmp(main_loop_jump, 1);
        syserr("longjmp\n");
    }
    /* Check if everything went correctly */
    if (current != NULL)
        fatal("There should be no current thread\n");
    if (!queue_is_empty(working) || !queue_is_empty(waiting))
        fatal("Thread queue should be empty");
}

