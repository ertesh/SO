#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "err.h"

int main ()
{
  register int i;

  if (setpgrp() == -1)  /*ustalenie nowej grupy*/
    syserr("setpgrp");

  for (i = 0; i < 10; i++)
    switch (fork()) {
      case -1:
        syserr("fork");
      case 0: /*proces potomny*/
        if (i & 1)
	  if (setpgrp() == -1)
	    syserr("setpgrp2");
        printf("pid = %d pgrp = %d\n", getpid(), getpgrp());
        if (i & 1)
	  sleep(15);
	else
	  pause();
	return 0;
    }

  sleep(5); /*dajmy czas procesom potomnym na rozpoczecie dzialania*/
  if (kill(0, SIGINT) == -1)
    syserr("kill");
  return 0;
}  /*main*/

