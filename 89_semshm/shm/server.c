#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#include "mesg.h"
#include "err.h"
#include "simple_sem.h"

int shmid, clisem, servsem;	/* shared memory and semaphore IDs */
Mesg *mesgptr;			/* ptr to message structure, which is
				   in the shared memory segment */

void server()
{
  int n, filefd;
  char errmesg[MAXMESGDATA * 2];

  P(servsem);  /* we'll wait here for client to start things */

  mesgptr->mesg_data[mesgptr->mesg_len] = '\0'; /* null terminate filename */

  if ( (filefd = open(mesgptr->mesg_data, 0)) < 0) {
    sprintf(errmesg, "server : can't open %s\n",mesgptr->mesg_data);
    strncpy(mesgptr->mesg_data, errmesg, MAXMESGDATA);
    errmesg[MAXMESGDATA - 1] = '\0';
    mesgptr->mesg_len = strlen(mesgptr->mesg_data);
    V(clisem);			/* send to client */
    P(servsem);			/* wait for client to process */
  } else {
    while ( (n = read(filefd, mesgptr->mesg_data, MAXMESGDATA-1)) > 0) {
      mesgptr->mesg_len = n;
      V(clisem);		/* send to client */
      P(servsem);		/* wait for client to process */
    }
    close(filefd);
    if (n < 0)
      syserr("server: read error");
  }
  mesgptr->mesg_len = 0;
  V(clisem);
}

int main()
{
  if ( (shmid = shmget(SHMKEY, sizeof(Mesg), PERMS | IPC_CREAT)) < 0)
    syserr("server: can't get shared memory");
  if ( (mesgptr = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1)
    syserr("server: can't attach shared memory");
  if ( (clisem = sem_initialize(SEMKEY1, IPC_CREAT)) < 0)
    syserr("server: can't create client semaphore");
  if ( (servsem = sem_initialize(SEMKEY2, IPC_CREAT)) < 0)
    syserr("server: can't create server semaphore");

  V(clisem);
  server();
  if (shmdt(mesgptr) < 0)
    syserr("server: can't detach shared memory");
  exit(0);
}
