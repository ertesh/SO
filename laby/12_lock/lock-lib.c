#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "err.h"
#include "lock-lib.h"
#include "rw-lib.h"

void blokuj(int desk, int typ)
{
  struct flock arg_blokady;
  char komunikat[MAXMESGSIZE];

  /* bedziemy zakladac blokade na caly plik, chociaz dopuszczalne jest
     zakladanie blokady na poszczegolne rekordy. Patrz funkcja lseek()
     na specyfikacje poszczegolnych argumentow */

  arg_blokady.l_whence = SEEK_SET; /* poczatek pliku*/
  arg_blokady.l_start = 0L;        /* zaczynamy od poczatku */ 
  arg_blokady.l_len = 0L;          /* blokujemy caly plik */
  
  /* probujemy az do skutku - mozliwe tylko dla blokad zalecanych
     (advisory) */ 

  while (1) {
  arg_blokady.l_type = typ;

  /* jezeli operacja sie udala wracamy do programu glownego; F_SETLK -
   operacja nieblokujaca vs. F_SETLKW - operacja blokujaca */

  if (fcntl(desk, F_SETLK, &arg_blokady) != -1) return;

  /* jezeli sie nie udalo sprawdzamy z kim mamy konflikt */

  if (errno != EAGAIN) syserr("fcntl - SETLK");
  else /* ktos inny utrzymuje zalozona blokade */
     if (fcntl(desk, F_GETLK, &arg_blokady) == -1)
       syserr("fcntl - GETLK");

  /* jest mozliwe, ze pomiedzy kolejnymi wywolaniami fcntl (F_SETLK i
     F_GETLK), ktos zwolnil blokade, dlatego sprawdzamy czy tak sie
     nie stalo */

  if (arg_blokady.l_type != F_UNLCK) 
    {
      sprintf(komunikat, "konflikt blokad z procesem %d \n nacisnij <Enter> aby dzialac dalej ...", arg_blokady.l_pid);
      akcja_uzytkownika(komunikat);
    }
  }
}
