#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include "err.h"
#include <setjmp.h>
jmp_buf buf;
void handler(int nr)
{
    printf("Handler\n");
    return;
    longjmp(buf, 1);
}


int main() {
    struct sigaction mysignal, oldsignal;
    mysignal.sa_handler = handler;
    sigemptyset(&mysignal.sa_mask);
    mysignal.sa_flags = 0;
    if (sigaction(SIGPROF, &mysignal, &oldsignal) == -1)
        syserr("sigaction\n");

    struct itimerval mytimer = {{0, 100000}, {0, 100000}};
    //mytimer.it_interval = mytimer.it_value;
    if (setitimer(ITIMER_PROF, &mytimer, NULL) == -1)
        syserr("sigaction\n");
    if (setjmp(buf) == -1)
        syserr("setjmp\n");
    while(1);
}

