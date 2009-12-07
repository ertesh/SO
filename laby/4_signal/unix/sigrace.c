#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "err.h"
#include "systemv.h"

void lapsygnal(int nr)  /* argumentem jest numer przechwyconego sygnalu */
{
  fprintf(stderr,"PID %d przechwycono sygnal nr %d\n", getpid(), nr);
  if (signal(nr, lapsygnal) == SIG_ERR)
    syserr("signal1");
}

int main()
{
   int ppid;

   if (signal(SIGINT, lapsygnal) == SIG_ERR)
     syserr("signal2");
   if (signal(SIGQUIT, lapsygnal) == SIG_ERR)
     syserr("signal3");

   switch(fork()) {
     case -1:
       syserr("fork");
     case 0: 
       ppid = getppid();
       for (;;)
         if (kill(ppid, SIGINT) == -1)
           syserr("kill");
   }

   /* nizszy priorytet, wieksza szansa na uzyskanie sytuacji wyscigu */
 	 if (nice (100) == -1)
	     syserr("nice");

   for (;;);
}  /*main*/
