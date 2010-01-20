#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include "err.h"
#include "lock-lib.h"
#include "rw-lib.h"

void blokuj(int desk, int typ)
{
  char komunikat[MAXMESGSIZE];
  int f_typ;

  /* bedziemy zakladac blokade na caly plik - blokady nie-posixowe; 
     probujemy az do skutku - mozliwe tylko dla blokad zalecanych (advisory) */ 

  switch (typ) {
  case F_UNLCK:
    f_typ = LOCK_UN;
    break;
  case F_RDLCK:
    f_typ = LOCK_SH | LOCK_NB; /* LOCK_NB - nie blokujaca */
    break;
  case F_WRLCK:
    f_typ = LOCK_EX | LOCK_NB;
    break;
  default:
    fatal("Nieznany typ blokady");
  }

  while (1) {           
  /* jezeli operacja sie udala wracamy do programu glownego; 
     LOCK_NB - operacja nieblokujaca */

  if (flock(desk, f_typ) != -1) return;

  /* jezeli sie nie udalo sprawdzamy z kim mamy konflikt */

  if (errno != EWOULDBLOCK) syserr("flock");
  else /* ktos inny utrzymuje zalozona blokade */
    {
      sprintf(komunikat, "konflikt blokad \n nacisnij <Enter> aby dzialac dalej ...");
      akcja_uzytkownika(komunikat);
    }
  }
}