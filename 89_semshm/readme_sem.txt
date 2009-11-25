Semafory w systemie Unix
========================

Literatura
----------

  * W. R. Stevens 
    "Programowanie w srodowisku systemu UNIX", rozdz. 15.8

  * W. R. Stevens 
    "Programowanie zastosowan sieciowych w systemie UNIX", rozdz. 3.10

  * M. J. Bach
    "Budowa systemu operacyjnego UNIX", rozdz. 11.2.3

  * M. Rochkind
    "Programowanie w systemie UNIX dla zaawansowanych", rozdz. 7.6

  * man do poszczegolnych funkcji

Programy przykladowe i pliki zrodlowe
-------------------------------------

Makefile
   Uniwersalny makefile.

err.c, err.h
   Obsluga bledow.

semun.h 
   Definicja unii semun (jesli system nie dostarcza tej definicji).

semtool.c
   Program pozwala na wykonanie wszystkich mozliwych dzialan na semaforach:
      - utworzyc zestaw semaforow;
      - zwykle operacje na semaforach, a wiec przechodzenie pod semaforem,
        podnoszenie semafora, etc.;
      - operacje "administracyjne" na semaforach.

simple_sem.h, simple_sem.c
   Biblioteka oferujaca klasyczne operacje P i V na semaforach.


1. Opis semaforow i dostepnych operacji
   ------------------------------------

   Semafory w systemie Unix (Linux) stanowia istotne uogolnienie klasycznych
   semaforow Dijkstry. Zestaw operacji na semaforach jest obszerny. Operacje sa
   wykonywane na zestawach semaforow. Semafory nalezace do danego zestawu sa
   ponumerowane kolejnymi liczbami naturalnymi, poczynajac od zera. Mozliwe
   jest wykonywanie jednoczesnych operacji na semaforach nalezacych do tego
   samego zestawu.

   Podstawowe funkcje systemowe przeznaczone do obslugi semaforow to:
      - semget - utworzenie zestawu semaforow
                  (uzyskanie dostepu do istniejacego zestawu semaforow)
      - semop  - operacje na semaforach
      - semctl - operacje administracyjne na zestawie semaforow

   Program w C, korzystajacy z mechanizmu semaforow, musi dolaczyc do programu
   pliki naglowkowe:

   #include <sys/types.h>
   #include <sys/ipc.h>
   #include <sys/sem.h>


1.1 Utworzenie (uzyskanie dostepu do) zestawu semaforow
    ---------------------------------------------------

   Chcac rozpoczac prace z semaforami nalezy utworzyc zestaw semaforow lub
   uzyskac dostep do juz istniejacego zestawu. Realizuje to funkcja
   systemowa semget:

      int semget (key_t klucz, int lsem, int flagi)

   Parametry 'klucz' oraz 'flagi' maja opisane w poprzednim scenariuszu
   znaczenie, wspolne dla wszystkich mechanizmow IPC.

   Z kolei parametr 'lsem' okresla liczbe semaforow w danym zestawie. Otwierajac
   istniejacy zestaw semaforow mozna podac dowolna liczbe nie wieksza od
   faktycznej liczby semaforow w danym zestawie (w szczegolnosci zero).

* Przeczytaj man semget

1.2 Operacje na zestawie semaforow
    ------------------------------

   Do wykonywania operacji na semaforach (podnoszenie, opuszczanie oraz
   oczekiwanie na zerowa wartosc semafora) sluzy funcja systemowa 'semop'.

      int semop (int ident, struct sembuf *operacje, unsigned loper)

   Znaczenie parametrow:
   * ident - identyfikator zestawu semaforow (przekazany przez 'semget')
   * loper - liczba operacji do jednoczesnego wykonania na semaforach
            nalezacych do danego zestawu,
            a rownoczesnie liczba elementow tablicy 'operacje',
            definiujacych operacje na poszczegolnych semaforach zestawu

   Wszystkie operacje okreslone w tablicy 'operacje' beda wykonane na
   podanych semaforach jako jedna niepodzielna operacja.

   Struktura 'sembuf' (zdefiniowana w pliku <sys/sem.h>) jest nastepujaca:

   struct sembuf {
      short int sem_num;		/* semaphore number */
      short int sem_op;		/* semaphore operation */
      short int sem_flg;		/* operation flag */
   };

   Opis jednej operacji obejmuje:
   * numer semafora w zestawie ('sem_num')
   * rodzaj operacji ('sem_op'), bedacy liczba calkowita, majacy
     nastepujace znaczenie:
      - wartosc dodatnia: zwiekszenie wartosci semafora (ewentualne
            uwolnienie czekajacych procesow),
      - wartosc ujemna: zadanie zmniejszenia wartosci semafora,
      - zero: sprawdzenie i ewentualne oczekiwanie az wartoscia semafora
            bedzie zero;
   * opcje ('sem_flg'), na przyklad:
      - IPC_NOWAIT - operacja nieblokujaca, tzn. jesli operacja nie moze
            byc wykonana natychmiast, to 'semop' natychmiast sie konczy
            (z wartoscia -1), a zmiennej 'errno' zostaje nadana wartosc EAGAIN,
      - SEM_UNDO - jesli operacja zmienia wartosc semafora, zmiana ta, w miare
            mozliwosci, zostanie zniwelowana w momencie zakonczenia wykonywania
            procesu. W Linuksie oznacza to, ze jesli semafor mialby uzyskac
            ujemna wartosc, zostanie zmniejszony do zera, po czym konczacy sie
            proces ostatecznie sie zakonczy.
               Obowiazek wykonania operacji UNDO nie jest dziedziczony w 'forku',
            podobnie jak wywolanie 'fork' nie wplywa na stan semafora.

   Nalezy pamietac, ze dopuszczenie jednoczesnych operacji semaforowych przez
   mechanizm SysV IPC powoduje, ze nie sa juz one zywotne. Na szczescie,
   poniewaz oczekujace procesy sa zorganizowane w kolejke FIFO, zywotnosc
   zostanie zachowana, jesli procesy wykonuja pojedyncze operacje modyfikujace
   semafor o 1.

   Wartoscia funkcji 'semop' jest zero, jesli jej wykonanie zakonczylo sie
   sukcesem, a -1 w przypadku bledu.

* Przeczytaj man semop

1.3 Operacje administracyjne na zestawie semaforow
    ----------------------------------------------

   Do wykonywania dodatkowych operacji na zestawie semaforow (ew. pojedynczych
   semaforach zestawu) sluzy funkcja systemowa 'semctl'.

      int semctl (int ident, int semnum, int polecenie, union semun arg));

   Znaczenie parametrow:
   * ident  - identyfikator zestawu semaforow (przekazany przez 'semget')
   * semnum - numer semafora, na ktorym ma byc wykonana operacja
   * arg    - parametry polecenia lub wartosci przekazywane przez 'semctl'
               (unia 'semun' jest zdefuniowana w pliku 'semun.h')
   * polecenie -
         GETVAL   - pobranie wartosc semafora
         SETVAL   - nadanie wartosci semaforowi (inicjacja, ale nie tylko) 
         GETPID   - przekazanie identyfikatora procesu, ktory jako
                     ostatni wykonal 'semop' na semaforze 'semnum'
         GETNCNT  - przekazanie liczby procesow czekajacych na
                     zwiekszenie wartosci semafora 'semnum'
         GETZCNT  - przekazanie liczby procesow czekajacych na
                     uzyskanie przez semafor 'semnum' zera
         GETALL   - pobranie wartosci wszystkich semaforow w zestawie
         SETALL   - nadanie wartosci wszystkim semaforom w zestawie
         IPC_STAT - pobranie informacji o zestawie semaforow
                     (m.in. identyfikator procesu, ktory ostatni wykonal
                     operacje na wskazanym semaforze, prawa dostepu,
                     liczba semaforow, czas ostatniej operacji/zmiany)
         IPC_SET  - zmiana niektorych atrybutow zestawu semaforow
                     (identyfikator wlasciciela/grupy, prawa dostepu)
         IPC_RMID - usuniecie zestawu semaforow
                     (obudzenie wszystkich czekajacych procesow - zmienna
                     'errno' ustawiana na EIDRM).

   Wartoscia funkcji jest przekazywana wartosc (polecenia GETVAL, GETPID,
   GETNCNT, GETZCNT) lub -1 w przypadku bledu, a wowczas zmienna 'errno'
   okresla rodzaj bledu. Podstawowe kody bledow:
      EACCES - proces nie ma prawa wykonac podanej operacji
      EINVAL - niepoprawna wartosc (rodzaj polecenia, identyfikator zestawu).

* Przeczytaj man semctl

2. Cwiczenie - poglebienie wiedzy o semaforach
   -------------------------------------------

   a) Sprawdz (o ile jeszcze tego nie wiesz) jakie wartosci poczatkowe sa
      nadawane semaforom.
   b) Rozpoznaj strategie obslugi operacji na semaforach.
      * najlepiej podzielic sie na male grupki i wspolnie przeprowadzic
         pewne testy
      * kolejnosc wykonywania klasycznych operacji P, V na jednym
         semaforze - czy jest to kolejka prosta, czy moze nastapic
         zaglodzenie procesu o nadmiernych wymaganiach?
      * kolejnosc wykonywania jednoczesnych operacji na kilku semaforach
   b) Nie zapomnij o usunieciu wszystkich utworzonych semaforow.

3. Semafory POSIX-owe
   ------------------

   Semafory POSIX-owe zawsze wystepuja pojedynczo i nie mozna, w
   przeciwienstwie do poznanego wyzej rozwiazania, latwo zapewnic atomowosci
   zlozonych operacji.

   Mamy do dyspozycji dwie rodziny semaforow (nazwane i nienazwane), ktore
   wspoldziela sposob obslugi operacji P i V, ale roznia sie metoda
   inicjalizacji i zamykania.

   Semafory nazwane sa widoczne w postaci plikow wirtualnego systemu plikow,
   ktorych nazwy, podobnie jak w przypadku kolejek POSIX-owych, sa postaci
   "/nazwa_semafora".

   Semafory nienazwane mozna natomiast traktowac jak zwykle, znajdujace sie
   w pamieci procesu zmienne, na ktorych dopuszczalne sa operacje podnoszenia
   oraz wstrzymujacego oczekiwania. Jest to ze wszech miar pozadane, gdyz
   pozwala zwiazac semafor z obiektem, ktory ma chronic.

   Aby moc wykorzystywac semafory nienazwane w taki sposob, trzeba jednak umiec
   utworzyc obszar pamieci dostepny dla wielu procesow. Zajmiemy sie tym
   w czesci poswieconej segmentom pamieci dzielonej.

* Przeczytaj man sem_overview

   Zarowno odpowiednie typy danych jak i operujace na nich funkcje sa
   zadeklarowane w pliku <semaphore.h>.

   Niestety, deklaracja deklaracja, a implementacja implementacja. Obecnie
   (jadro w wersji 2.6.22.19), zaimplementowane sa tylko semafory nienazwane.

   Pokrotce przedstawimy teraz sposob korzystania z nich. We wszystkich
   przypadkach, poprawnie konczace sie funkcje przekazuja wartosc 0, zas
   w wypadku bledu zwracaja wartosc -1 oraz odpowiednio ustawiaja zmienna
   'errno'.

3.1 Tworzenie semaforow nienazwanych
    --------------------------------

   Semafor jes zmienna typu 'sem_t', ktora nalezy zainicjowac, przed wykonaniem
   pierwszej operacji semaforowej, za pomoca funkcji 'sem_init':

   #include <semaphore.h>
   int sem_init(sem_t *sem, int pshared, unsigned int value);

   przy czym 'sem' jest wskaznikiem do inicjowanego semafora, 'pshared' musi byc
   rozne od 0, jesli semafor ma byc uzywany przez wiecej niz jeden proces
   (wartosc 0 odpowiada semaforowi wspoldzielonemu przez watki jednego procesu),
   zas 'value' oznacza poczatkowy stan semafora.

   Wykonywanie operacji semaforowych na niezainicjowanym semaforze albo
   wielokrotna inicjalizacja tego samego semafora moga prowadzic do
   nieokreslonego zachowania.

3.2 Operacje semaforowe
    -------------------

   Operacje semaforowe wykonuje sie za pomoca funkcji 'sem_post' oraz rodziny
   funkcji 'sem_wait':

   #include <semaphore.h>

   /* opuszczenie semafora */
   int sem_wait(sem_t *sem);
   int sem_trywait(sem_t *sem);
   int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

   /* podniesienie semafora */
   int sem_post(sem_t *sem);

   Wszystkie funkcje zmieniaja wartosc semafora tylko o 1. Ponadto opuszczenie
   semafora jest z definicji operacja blokujaca, przy czym w zaleznosci od
   konkretnej funkcji, rozne moze byc zachowanie w przypadku niemoznosci
   natychmiastowego wykonania tej operacji.

   W przypadku funkcji 'sem_trywait' oznacza to powrot z bledem, zas dla funkcji
   'sem_timedwait' zawieszenie sie w oczekiwaniu do uplyniecia chwili okreslonej
   przez drugi z przekazanych argumentow.

   Oprocz tych, istnieje rowniez funkcja 'sem_getvalue', pozwalajaca poznac
   aktualna wartosc semafora.

3.3 Zniszczenie semafora nienazwanego
    ---------------------------------

   Semafor zainicjowany za pomoca funkcji 'sem_init' nalezy zniszczyc, gdy juz
   nie jest potrzebny, za pomoca funkcji 'sem_destroy'. W ten sposob zwalniamy
   zasoby systemowe, zwiazane z semaforem.

   #include <semaphore.h>
   int sem_destroy(sem_t *sem);

   Zniszczenie semafora, na ktorym czekaja inne procesy, prowadzi do
   nieokreslonego zachowania.

* Przeczytaj odpowiednie strony mana

   UWAGA: Poki co, w celu skorzystania z semaforow, nalezy swoje programy
      linkowac z biblioteka 'librt', tzn. do flag linkera (zwyczajaowo LDFLAGS)
      dopisac -lrt
