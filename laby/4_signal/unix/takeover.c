#include <stdio.h>
#include <signal.h>
#include "err.h"
#include "systemv.h"
/* You may want to include this "systemv.h" to enforce System V semantics
   for signal. 
   #include "systemv.h"
*/

void takeover(int k) 
{
  printf("Signal %d caught.\n", k);
}

int main() 
{
   if (signal(SIGINT, takeover) == SIG_ERR)
     syserr("signal");
   
   for(;;);
}
