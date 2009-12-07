#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "err.h"

int main (int argc, char *argv[])
{
  int i;
  
  if (argc > 1)
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) /*ignoruj sygnal smierci potomka*/
      syserr("signal");             

  for (i = 0; ; i++)
    switch (fork()) {
      case -1:
        syserr("fork");
      case 0:
	printf("child: no     = %2d, PID = %7d\n", i, getpid());
	exit(i);
    }
  
  return 0;
}  /*main*/
