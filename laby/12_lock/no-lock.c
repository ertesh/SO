#include <stdio.h>
#include <fcntl.h>
#include "rw-lib.h"
#include "err.h"

int main(int argc, char *argv[])
{
  int desk;

  /* potrzebujemy nazwy pliku do czytania-pisania */ 

  if (argc != 2)
    fatal("uzycie %s nazwa_pliku", argv[0]); 

  if ((desk = open(argv[1], O_RDWR | O_CREAT, 0666)) == -1) 
    syserr("open");

  printf("* proba czytania...\n");
  
  czytaj(desk);

  akcja_uzytkownika("nacisnij <Enter> aby dzialac dalej...");

  printf("* proba pisania...\n");
  
  pisz(desk);

  akcja_uzytkownika("nacisnij <Enter> aby wyjsc...");

  return 0;
}