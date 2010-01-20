#include <stdio.h>
#include <unistd.h>
#include "err.h"
#include "rw-lib.h"
#include "string.h"

/* interakcja z uzytkownikiem */
void akcja_uzytkownika (char *komunikat)
{
  char buf[MAXBUFSIZE];

  fprintf(stderr,"%s\n", komunikat);

  if (!fgets(buf, MAXBUFSIZE-1, stdin)) 
    syserr("fgets");
}

void pisz(int desk)
{
  char dane[MAXDATASIZE]; 

  printf("Podaj dane:\n >> ");

  if (!fgets(dane, MAXDATASIZE-1, stdin))
    syserr("fgets");
  
  if (write(desk, dane, strlen(dane)) == -1) 
      syserr("write");
}

void czytaj(int desk)
{
  char dane[MAXDATASIZE]; 
  int ile;  

  switch (ile=read(desk, dane, MAXDATASIZE)) {
  case -1:
    syserr("read");
  case 0:
    fprintf(stderr,"Pusty plik!\n");
    break;
  default:
    dane[ile]='\0';
    printf("Przeczytalem: %s\n", dane);
  }
}
