#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "mesg.h"
#include "err.h"
#include "simple_sem.h"

int shmid, clisem, servsem;	/* shared memory and semaphore IDs */
Mesg *mesgptr;			/* ptr to message structure, which is
				   in the shared memory segment */
void client()
{
  int n;

  P(clisem);			/* get control of shared memory */

  printf("Enter filename: ");
  if (fgets(mesgptr->mesg_data, MAXMESGDATA, stdin) == NULL)
    syserr("filename read error");

  n = strlen(mesgptr->mesg_data);
  if (mesgptr->mesg_data[n - 1] == '\n')
    n--;			/* ignore newline from fgets() */
  mesgptr->mesg_len = n;
  V(servsem);			/* wake up server */

  P(clisem);			/* wait for server to process */
  while( (n = mesgptr->mesg_len) > 0) {
    if (write(1, mesgptr->mesg_data, n) != n)
      syserr("data write error");
    V(servsem);			/* wake up server */
    P(clisem);			/* wait for server to process */
  }

  if (n < 0)
    syserr("data read error");
}

int main()
{
  if ( (shmid = shmget(SHMKEY, sizeof(Mesg), 0)) < 0)
    syserr("client: can't get shared memory segment");
  if ( (mesgptr = (Mesg *) shmat(shmid, 0, 0)) == (Mesg *) -1)
    syserr("client: can't attach shared memory segment");

  if ( (clisem = sem_initialize(SEMKEY1, IPC_CREAT)) < 0)
    syserr("client: can't open client semaphore");
  if ( (servsem = sem_initialize(SEMKEY2, IPC_CREAT)) < 0)
    syserr("client: can't open server semaphore");

  client();

  if (shmdt(mesgptr) < 0)
    syserr("client: can't detach shared memory");
  if (shmctl(shmid, IPC_RMID, 0) < 0)
    syserr("client: can't remove shared memory");

  sem_done(clisem);		/* will remove the semaphore */
  sem_done(servsem);  		/* will remove the semaphore */

  return 0;
}
