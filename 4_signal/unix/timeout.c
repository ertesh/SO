#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "err.h"
#include "systemv.h"

void przeterminowanie (int typ_sygnalu) {}

int t_read(int fd, char *buf, int bufsize, int timeout)
{
  int ret;
  void (*handler)(int);
 
  if ((handler = signal(SIGALRM, przeterminowanie)) == SIG_ERR)
    syserr("signal1");

  alarm(timeout);
  ret = read(fd, buf, bufsize);
  alarm(0);

  if (signal(SIGALRM, handler) == SIG_ERR)
    syserr("signal2");

  return ret;
}

int main() 
{
  char bufor[100];
  int wynik;

  setvbuf(stdout, 0, _IONBF, 0);
  
  while(1) {
    printf("wprowadz napis: ");
    switch (wynik = t_read(0, bufor, sizeof(bufor), 5)) {
      case -1:
        if (errno == EINTR) {
          printf("czas minal!\n");
          break;
        } else
          syserr("t_read");
      case 0:
	return 0;
      default:
        bufor[wynik - 1] = '\0';   /* koniec napisu zamiast konca linii */
        printf("wprowadzono napis: %s\n", bufor);
    }
  }
}
