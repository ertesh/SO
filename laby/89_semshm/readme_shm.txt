Pamiec dzielona w systemie Linux
================================

Literatura:
  * W. R. Stevens 
    "Programowanie w srodowisku systemu UNIX", rozdz. 15.9

  * W. R. Stevens 
    "Programowanie zastosowan sieciowych w systemie UNIX", rozdz. 3.10

  * M. J. Bach
    "Budowa systemu operacyjnego UNIX", rozdz. 11.2.2

  * M. Rochkind
    "Programowanie w systemie UNIX dla zaawansowanych", rozdz. 7

  * man do poszczegolnych funkcji

Programy przykladowe i pliki zrodlowe
-------------------------------------
  a) 'simple_sem' - biblioteka oferujaca klasyczne operacje P i V na semaforach.

  b) 'msgsrv' i 'msgcli' - programy umozliwiajace przesylanie plikow przez 
      pamiec dzielona, analog programu z zajec o kolejkach komunikatow.
      Nalezy uruchomic 'msgsrv' na jednej konsoli, a 'msgcli' na drugiej.
      'msgcli' pobiera nazwe pliku z stdin.

      Programy te maja zdefiniowany klucz w pliku 'mesg.h'.

  c) 'datex' - program pokazujacy wymiane zlozonej informacji poprzez pamiec
      dzielona oraz niuanse inicjalizacji struktur w takiej pamieci.

  d) 'posix_philosem' - program symulujacy filozofow za pomoca nienazwanych
      semaforow POSIX-owych, umieszczonych w pamieci dzielonej


Pliki zawierajace zrodla programow przykladowych.

   Makefile
      Uniwersalny makefile.

   err.c, err.h
      Obsluga bledow.

   semun.h 
      Definicja unii semun (jesli system nie dostarcza tej definicji).

   simple_sem.h
      Plik naglowkowy dla 'simple_sem.c'. Deklaracje dostepnych funkcji.

  simple_sem.c
      Implementacja prostych semaforow. Domyslnie nie tworzy semaforow, a
      wszystkie operacje sa oznaczane flaga SEM_UNDO, co przynajmniej dla
      semaforow traktowanych jak muteksy, zapewnia zywotnosc mechanizmu.

   mesg.h
      Plik naglowkowy dla 'msgsrv.c' i 'msgcli.c'. Definicja komunikatu
      i klucza kolejki.

   msgsrv.c
      Serwer sluzacy do wysylania plikow. W petli odbiera nazwy plikow 
      od klientow i przesyla te pliki klientom. Przechwytuje SIGINT, 
      ktorego wywolanie powoduje zakonczenie programu.

   msgcli.c
      Klient czyta z wejscia nazwe pliku i wysyla serwerowi. Odebrany
      plik wypisuje na stdout.

   datex.c
      "Pierwsze" wywolanie programu tworzy i inicjalizuje wewnetrzne struktury
      danych w pamieci dzielonej. Zakonczone przez przechwytywany sygnal SIGINT,
      usuwa wspoluzytkowane zasoby.

   posix_philosem.c
      Tworzy segment pamieci dzielonej i inicjuje w nim wszystkie semafory,
      a nastepnie forkuje sie odpowiednia liczbe razy. Konczy sie po otrzymaniu
      sygnalu SIGINT.
         Zaimplementowane sa w nim dwie funkcje tworzace pamiec dzielona, ale
      w chwili obecnej (jadro w wersji 2.6.22.19) mozna korzystac tylko z
      mechanizmow SysV IOC, gdyz POSIX-owe nie sa zaimplementowane.


1. Opis segmentow pamieci dzielonej i dostepnych operacji
   -------------------------------------------------------

   W systemie Unix kazdy proces wykonuje sie we wlasnej wirtualnej przestrzeni 
   adresowej. Przestrzenie roznych procesow sa rozlaczne, co stwarza
   problemy ze wspoldzieleniem informacji (nie mozna zdefiniowac
   zmiennych "globalnych", ktore bylyby dostepne dla wielu procesow). W
   celu rozwiazania tego problemu do pakietu IPC Systemu V dolaczono
   mechanizm segmentow pamieci dzielonej. 

   Segment pamieci dzielonej jest fragmentem pamieci. Zarzadza nim system
   operacyjny. Segment pamieci dzielonej sklada sie z pewnej liczby stron
   (a zatem jego rozmiar jest zawsze wielokrotnoscia rozmiaru
   strony). Segment pamieci dzielonej moze na zadanie procesu byc
   przylaczony pod wskazany lub wybrany przez system operacyjny (to
   drugie rozwiazanie jest preferowane) adres wirtualny procesu. Jesli
   zatem dwa procesy przylacza ten sam segment do wlasnych przestrzeni
   adresowych to wszystkie dane znajdujace sie w tym segmencie staja sie
   dostepne dla obydwu procesow. W ten sposob uzyskuje sie wspoldzielenie
   danych. 

   Po przylaczeniu segmentu do pamieci wirtualnej procesu dostep do
   danych znajdujacych sie w nim odbywa sie za pomoca zwyklych operacji
   dostepu do pamieci. Daje to duza wydajnosc tego mechanizmu
   komunikacji. Niestety te zyski ze stosowania pamieci dzielonej sa
   czesto rownowazone przez narzut zwiazany z koniecznosci synchronizacji
   dostepu do danych wspoldzielonych (najczesciej za pomoca semaforow). 

   Gdy segment pamieci staje sie juz niepotrzebny w danym procesie,
   nalezy go odlaczyc od jego przestrzeni adresowej. Po odlaczeniu,
   system operacyjny w dalszym ciagu przechowuje segment i dane w nim
   zapisane (nawet, jesli segment nie jest juz przylaczony do zadnego
   procesu). Gdy segment staje sie juz niepotrzebny nalezy go usunac.

   Podstawowe funkcje systemowe przeznaczone do obslugi segmentow pamieci
   dzielonej to:
      - shmget - utworzenie segmentu pamieci dzielonej
                  (uzyskanie dostepu do istniejacego segmentu)
      - shmat  - przylaczenie segmentu pamieci dzielonej
      - shmdt  - odlaczenie segmentu pamieci dzielonej
      - shmctl - operacje administracyjne na segmencie pamieci dzielonej

   Program w C, korzystajacy z mechanizmu pamieci dzielonej, musi dolaczyc do
   programu pliki naglowkowe:

   #include <sys/types.h>
   #include <sys/ipc.h>
   #include <sys/shm.h>


1.1 Utworzenie (uzyskanie dostepu do) segmentu pamieci dzielonej
    ------------------------------------------------------------

   Operacja ta przypomina analogiczne operacje wykonywane na kolejkach
   komunikatow i zestawach semaforow.

   Utworzenie (lub uzyskanie dostepu do juz istniejacego segmentu)
   realizuje funkcja systemowa shmget:

     int shmget (key_t klucz, int rozmiar, int flagi)

   Parametr 'rozmiar' okresla rozmiar tworzonego segmentu pamieci
   dzielonej i jest istotny tylko wowczas, gdy funkcja faktycznie tworzy
   nowy segment. Nalezy pamietac, ze rzeczywisty rozmiar segmentu bedzie 
   najmniejsza wielokrotnoscia rozmiaru strony wieksza od wskazanego
   rozmiaru. 

   Funkcja 'shmget' przekazuje identyfikator segmentu pamieci dzielonej
   (albo -1 w przypadku bledu).

* Przeczytaj man shmget


1.2 Przylaczanie segmentu pamieci dzielonej
    ---------------------------------------

   Aby proces mogl skorzystac z segmentu pamieci dzielonej musi:
   - utworzyc (lub otworzyc istniejacy segment)
   - przylaczyc go do swojej przestrzeni adresowej

   Do wykonywania operacji przylaczania sluzy funkcja systemowa 'shmat'.

     void *shmat (int ident, const void *adres, int flagi)

   Znaczenie parametrow:
      ident - identyfikator segmentu (przekazany przez 'shmget')
      adres - sugerowany adres wirtualny, pod ktory nalezy przylaczyc 
              segment
      flagi - rozne znaczniki kontrolujace sposob wykonania przylaczenia

   Funkcja 'shmat' dziala nastepujaco:
   - jesli 'adres' = 0, to system przylacza segment pamieci o
      identyfikatorze 'ident' pod wybrany przez siebie, dogodny adres
      wirtualny. Adres ten jest przekazywany jako wynik funkcji. 
   - jesli 'adres' <> 0, to system probuje przylaczyc segment pamieci pod 
      podany adres wirtualny. Adres musi byc wielokrotnoscia wartosci
      SHMLBA, chyba ze wsrod flag uzyto opcji SHM_RND, ktora wymusza 
      automatyczne dostosowanie podanej wartosci adresu do
      wielokrotnosci SHMLBA. Wynikiem funkcji jest rzeczywisty adres
      dokonania przylaczenia.
   - jesli wsrod flag uzyto opcji SHM_RDONLY przylaczony segment mozna
      jedynie odczytywac.
   - jesli funkcja shmat nie powiodla sie, to jej wynikiem jest 
      (void *) -1

   Uwaga. Preferowana jest metoda z wartoscia 'adres' = 0. Programista nie
   musi wtedy znac szczegolow organizacji pamieci (ktore w zaleznosci od
   wariantu systemu moga byc rozne), uzyskuje przenosny program. Niektore
   systemy wrecz ignoruja wartosc parametru adres i dokonuja
   przylaczenia tam, gdzie im jest wygodnie.

* Przeczytaj man shmat

   Przyklad:
      int   id;
      char* adr;

      if ((id=shmget (KLUCZ, 100, 0666|IPC_CREAT)) == -1)
      syserr ("blad");
      if ((adr=shmat(id, 0, 0)) < 0)
      syserr ("blad")

   Teraz wszystkie operacji odbywajace sie na napisie 'adr' 
   (np. strcpy (adr, "ala ma kota")) beda dotyczyc segmentu pamieci 
   dzielonej. 

1.3 Odlaczanie segmentu pamieci dzielonej
    -------------------------------------

   Gdy segment przestaje byc potrzebny w procesie nalezy go odlaczyc
   wywolujac funkcje systemowa 'shmdt':

     int shmdt (const void *adres)

   Argumentem funkcji jest adres, pod ktorym podpieto segment pamieci
   dzielonej, a nie identyfikator. Jest to spowodowane tym, ze proces
   moze podpiac segment pamieci dzielonej w kilka roznych miejsc swojej
   przestrzeni adresowej. Wtedy adres jednoznacznie okresla, o ktore
   podpiecie chodzi a identyfikator nie daje pelnej informacji

   Odlaczone segmenty nie gina nawet, jesli nie sa podpiete do zadnego
   procesu.

* Przeczytaj man shmdt

1.4 Usuwanie segmentu pamieci dzielonej
    -----------------------------------

   W celu usuniecia segmentu pamieci dzielonej wykonuje sie funkcje:
      int shmctl (int ident, IPC_RMID, 0)

   Segment nie jest jednak usuwany natychmiast, a jedynie oznaczany do
   usuniecia. Fizyczne usuniecie nastapi dopiero wtedy, gdy nie bedzie on
   juz przylaczony do przestrzeni adresowej zadnego procesu.

   Co ciekawe, poslugujac sie identyfikatorem takiego przeznaczonego do usuniecia
   segmentu, mozna w dalszym ciagu przylaczac go do nowych procesow. Proba
   posluzenia sie uniewaznionym juz kluczem doprowadzilaby do utworzenia nowego
   segmentu lub bledu, w zaleznosci od flag przekazanych funkcji 'shmget'.

* Przeczytaj man shmctl

1.5 Segmenty pamieci dzielonej a funkcje systemowe fork, exec, exit
    ---------------------------------------------------------------

   Po wykonaniu funkcji 'fork' potomek dziedziczy wszystkie przylaczone
   segmenty pamieci dzielonej. Kazdy z nich jest zatem przylaczony do
   dwoch procesow: ojca i potomka.

   Po wykonaniu funkcji 'exec' i 'exit' wszystkie przylaczone segmenty
   pamieci dzielonej sa automatycznie odlaczane (ale nie usuwane).


2. Cwiczenie
   ---------

   a) Przypomnij sobie przyklad serwera i klienta z laboratorium o kolejkach
      komunikatow.
   b) Zastanow sie, jak mozna zaimplementowac komunikacje w tym zadaniu za
      pomoca semaforow oraz segmentow pamieci dzielonej.
   c) Przeczytaj i przeanalizuj tresc programow 'client.c', 'server.c'.


3. Cwiczenie --- niuanse inicjalizacji i adresowania w pamieci dzielonej
   ---------------------------------------------------------------------

   Program 'datex.c' prezentuje trudnosci, na jakie napotykaja procesy w
   sytuacji, gdy przed rozpoczeciem wspoldzialania, trzeba w dzielonej pamieci
   zainicjalizowac jakies struktury.

   a) Przeczytaj kod programu 'datex.c', zwracajac uwage na sposob
      synchronizacji dostepu do inicjowanego obszaru pamieci. Jak by sie
      zmienilo zachowanie programu, gdyby procesy rozstrzygaly, ktory z 
      nich ma zainicjowac pamiec na podstawie zachowania funkcji 'shmget'
      albo w innej kolejnosci zwalnialy zasoby?
   b) Uruchom jednoczesnie dwa procesy 'datex'. Wytlumacz zachowanie tego,
      ktory uzyskuje dostep do pamieci jako drugi, tzn. wypisuje informacje
      na ekran.
   c) Rozwaz zastapienie odpowiednich linii przez zakomentowane linie
      zaczynajace sie od sekwencji /**///. Na czym polega zmiana "logiki"
      programu?

4. Zadanie
   -------

   Zaprojektuj prosta biblioteke pozwalajaca na tworzenie list (np. krotkich
   8-znakowych slow) w pamieci dzielonej. Powinna ona pozwalac na przegladanie
   listy, dodawanie/usuwanie elementu z listy oraz na zmiane wartosci
   przechowywanej w danym elemencie.

   Pamietaj o tym, by zabezpieczyc liste przed jednoczesnymi operacjami
   modyfikujacymi ja.

   Mozesz w tym celu skorzystac z semaforow nienazwanych, omowionych
   w poswieconej semaforom POSIX-owym czesci poprzedniego scenariusza.

5. Pamiec wspoldzielona wg standardu POSIX
   ---------------------------------------

   UWAGA: W chwili obecnej (jadro w wersji 2.6.22.19) zgodna ze standardem
      obsluga mechanizmu pamieci dzielonej nie jest zaimplementowana.
      Ponizszy opis jest wiec tylko opisem standardu.

   Poniewaz pamiec wspoldzielona jest najbardziej zblizonym do warstwy
   sprzetowej mechanizmem synchronizujacym, nie ma istotnych roznic pomiedzy
   pamiecia przydzielona w ramach SysV IPC, a ta oferowana przez POSIX-owe
   wywolanie 'shm_open'. Poniewaz mechanizm POSIX-owy, zamiast uzycia 'shmat',
   wymaga uzycia funkcji 'mmap', pozwala w zwiazku z tym na uzyskanie dostepu
   do pamieci w trybie kopiowania przy zapisie (Copy-On-Write).

* Przeczytaj man shm_open
