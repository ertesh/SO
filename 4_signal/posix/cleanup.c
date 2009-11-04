#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "err.h"

void catch_segv(int sig)
{
  sigset_t block_mask;
    
  printf("Cleaning up...");
  while (getchar() != '\n');

  sigemptyset(&block_mask);
  sigaddset(&block_mask, SIGSEGV);
  if (sigprocmask(SIG_UNBLOCK, &block_mask, 0) == -1)
    syserr("sigprocmask"); 
  if (raise(SIGSEGV) == -1)
    syserr("raise");
}

int main ()
{
  char *text = "Dummy";
  struct sigaction setup_action;
  sigset_t block_mask;

  sigemptyset(&block_mask);
  sigaddset(&block_mask, SIGINT);
  sigaddset(&block_mask, SIGQUIT);
  setup_action.sa_handler = catch_segv;
  setup_action.sa_mask = block_mask;
  setup_action.sa_flags = SA_ONESHOT;
  if (sigaction (SIGSEGV, &setup_action, 0) == -1)
    syserr("sigaction2");

  *text = 'Y';     /* 'calling' SIGSEGV :-) */
  return 0;
}
