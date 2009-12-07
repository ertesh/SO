#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#include "simple_sem.h"
#include "err.h"

#define MAX_MSG_SIZE 4000

void do_nothing(int sig) { }

void register_int_handler(void) {
    struct sigaction setup_action;
    sigset_t block_mask;

    sigemptyset (&block_mask);
    setup_action.sa_handler = do_nothing;
    setup_action.sa_mask = block_mask;
    setup_action.sa_flags = SA_RESETHAND;
    if (sigaction (SIGINT, &setup_action, 0) == -1)
        syserr("sigaction");
}

typedef struct ShortStruct {
/**///    int smp;
    char *smp;  /* ShortMessage Pointer */
    char shortMsg[MAX_MSG_SIZE];
} ShortStruct;

int main (void) {
    key_t ipc_key = 0x12345;
    int semid, shmid;
    ShortStruct *ssaddr;

    /* Single-Initialization semaphore's acquisition */
    if ((semid = sem_initialize(ipc_key, IPC_CREAT|IPC_EXCL)) == -1)
        if (errno != EEXIST) syserr("semget error");
        else {  
            /* semaphore's already created -- just wait for 
               memory initialization and V(semid) */
            if ( (semid = sem_initialize(ipc_key, 0)) == -1 )
                syserr("semget error");
            P(semid);   
            /* wait with returning the semaphore until memory 
               is attached --- it is "unremovable" then */
            if( (shmid = shmget(ipc_key, sizeof(ShortStruct), 0)) < 0)
                syserr("shmget error");
            if( (ssaddr = (ShortStruct*)shmat(shmid, 0, 0)) < 0)
                syserr("shmat error");
            V(semid);  
            /* open semaphore to allow other processes to use 
	       the shared data */

            printf ("I am pid = %d process.  Messages in the shared memory (shmid = %d):\n 1) %75s\n 2) %75s\n",
/**///                  getpid(), shmid, ssaddr->shortMsg, (char*)ssaddr + ssaddr->smp);
                        getpid(), shmid, ssaddr->shortMsg, ssaddr->smp);
            /* memory will be detached on exit */
        }
    else {  /* we are to initialize memory and then open semaphore */
        time_t ct;
        register_int_handler();

        if( (shmid = shmget(ipc_key, sizeof(ShortStruct), 0666 | IPC_CREAT | IPC_EXCL)) < 0)
            syserr("shmget error");
        if( (ssaddr = (ShortStruct*)shmat(shmid, 0, 0)) < 0)
            syserr("shmat error");

        time(&ct);
        sprintf(ssaddr->shortMsg, "Shared memory segment created by process pid = %d on %s",
                    getpid(), asctime(localtime(&ct)));
/**///        ssaddr->smp = ssaddr->shortMsg-(char*)ssaddr;
        ssaddr->smp = ssaddr->shortMsg;
        V(semid);

        /* waiting until SIGINT caught */
        pause();

        P(semid);
        shmctl(shmid, IPC_RMID, 0);
        sem_done(semid);
    }

    return 0;
}
