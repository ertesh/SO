#include <signal.h>
#include <stdio.h>
#include "err.h"

void catch_int (int sig) { printf("Zlapano INT\n" ); }

int main ()
{
  struct sigaction setup_action;
  sigset_t block_mask, pending_mask;

  sigemptyset (&block_mask);
  setup_action.sa_handler = catch_int;
  setup_action.sa_mask = block_mask;
  setup_action.sa_flags = 0;
  if (sigaction (SIGINT, &setup_action, 0) == -1)
    syserr("sigaction");
  printf("Wcisnij kilka razy ctrl-C, a potem enter\n");
  while (getchar() != '\n') ;

  sigaddset(&block_mask, SIGINT);
  if (sigprocmask(SIG_BLOCK, &block_mask, 0) == -1)
    syserr("sigprocmask1");
  printf("Wcisnij kilka razy ctrl-C, a potem enter\n");
  while (getchar() != '\n') ;

  if (sigpending(&pending_mask) == -1)
    syserr("sigpendind");
  if (sigismember(&pending_mask, SIGINT))
    printf("Czeka na swoja kolej SIGINT\n");

  if (sigprocmask(SIG_UNBLOCK, &block_mask, 0) == -1)
    syserr("sigprocmask2");
  return 0;
}
