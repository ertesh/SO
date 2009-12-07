#include <signal.h>
#include <stdio.h>
#include "err.h"

void a_handler (int signum)
{
  psignal (signum, "Caught signal");
}

int main ()
{
  int i;
  struct sigaction old, new;
    
  new.sa_handler = a_handler;
  sigemptyset(&new.sa_mask); 
  new.sa_flags = 0;

  for (i = 1; i < NSIG; i++) {
    if (sigaction(i, 0, &old) == -1)
      syserr("sigaction");

    printf("Sig %d - handler: %s, flags: %x, text: %s\n", i, 
           old.sa_handler == SIG_DFL ? "SIG_DFL" : "SIG_IGN", 
           old.sa_flags, sys_siglist[i]);

    if (sigaction(i, &new, 0) == -1)
      printf( "You can\'t ovveride signal %d handler.\n", i);
  }
  while(getchar() != '\n');
  return 0;
}
