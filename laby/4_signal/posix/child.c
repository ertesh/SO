#include <signal.h>
#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <sys/types.h>
#include "err.h"

void catch_child (int sig)
{
  int status;
  pid_t cpid;

  if ((cpid = wait(&status)) == -1)
    syserr("wait");
  printf("\nProcess %d exited with code %d\n", cpid, WEXITSTATUS(status));
}

int main ()
{
  struct sigaction setup_action;
  sigset_t block_mask;
  char buf[100];
  int size;
  
  sigemptyset (&block_mask);
  setup_action.sa_mask = block_mask;
  setup_action.sa_flags = 0;
  setup_action.sa_handler = catch_child;
  sigaction (SIGCHLD, &setup_action, 0);

  if (siginterrupt(SIGCHLD, 0) == -1)
    syserr("siginterrupt");

  switch (fork()) {
    case -1:
      syserr("fork");
    case 0:
      sleep(2);
      return getpid() % 10;
    default:
      setvbuf(stdout, (char *)0, _IONBF, 0);
      printf("Enter text: ");
      if ((size = read(0, buf, sizeof(buf) - 1)) == -1)
	syserr("read");
      buf[size] = '\0';
      printf("Read: %s", buf); 
      return 0;
  }
}
