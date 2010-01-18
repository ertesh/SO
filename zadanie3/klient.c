#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sched.h"
#define ILE 13

void fun() {
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
        printf("%s\n", nazwa);
		watki[i] = mysched_create_thread(fun, nazwa);
	}
	mysched_go();
	return 0;
}

