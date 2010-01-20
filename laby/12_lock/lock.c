#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "rw-lib.h"
#include "lock-lib.h"
#include "err.h"

/* Zaloz (lub zdejmij) blokade odpowiedniego typu na deskryptorze pliku
   przekazanym jako parametr. Typem moze byc F_UNLOCK lub F_RDLOCK lub
   F_WRLOCK */ 

int main(int argc, char *argv[])
{
  int desk;

  /* potrzebujemy nazwy pliku do zablokowania */ 

  if (argc != 2)
    fatal("uzycie %s nazwa_pliku", argv[0]); 

  if ((desk = open(argv[1], O_RDWR | O_CREAT, 0666)) == -1) 
    syserr("open");

  /* jezeli blokada ma byc obowiazkowa to trzeba dla pliku ustawic bit
     set-group-id */

#ifdef MANDATORY
  if (fchmod(desk, S_ISGID | 0666) == -1)
    syserr("fchmod");
#endif
    
  printf("* probuje zalozyc blokade dzielona - do czytania...\n");
  blokuj(desk, F_RDLCK);  
  printf("* zalozylem blokade do czytania...\n");
  
  czytaj(desk);

  akcja_uzytkownika("nacisnij <Enter> aby dzialac dalej...");

  printf("* zwalniam blokade\n");
  blokuj(desk, F_UNLCK);

  printf("* probuje zalozyc blokade wylaczna - do pisania...\n");
  blokuj(desk, F_WRLCK);  
  printf("* zalozylem blokade do pisania...\n");
  
  pisz(desk);

  akcja_uzytkownika("nacisnij <Enter> aby wyjsc...");

  /* koniec dzialania - blokady sa automatycznie zwalniane, gdy plik
     jest zamykany */

#ifdef MANDATORY
  if (fchmod(desk, 0666) == -1)
    syserr("fchmod");
#endif

  printf("* wychodze, wiec zwalniam blokade do pisania...\n");  

  return 0;
}