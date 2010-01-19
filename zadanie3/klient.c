#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sched.h"
#define ILE 3

void fun() {
    mysched_thread_t id = mysched_self();
    printf("%s\n", mysched_get_name(id));
    int limit = 100000000;
    int a = 7 + id;
    int mod = 1009;
    int i;
    for (i = 0; i < limit; i++)
        a = (a * a) % mod;
    printf("Koniec funkcji %d: %d\n", id, a);
}

void create_name(char* nazwa, int numer)
{
    strcpy(nazwa, "funkcja: ");
    char buffer[5];
    sprintf(buffer, "%d", numer);
    strcpy(nazwa+9, buffer);
}

int main() {
	int i;
	mysched_init();
	mysched_thread_t watki[ILE];
	for (i = 0; i < ILE; i++) {
		char nazwa[20];
        create_name(nazwa, i);
		watki[i] = mysched_create_thread(fun, nazwa);
	}
	mysched_go();
	return 0;
}

