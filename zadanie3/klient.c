#include <stdlib.h>
#include "sched.h"
#define ILE 10

void fun() {
}

int main() {
	int i;
	mysched_init();
	mysched_thread_t watki[ILE];
	for (i = 0; i < ILE; i++) {
		char* nazwa = "funkcja  ";
		nazwa[8] = ('0' + i);
		
		watki[i] = mysched_create_thread(fun, nazwa);
	}
	mysched_go();
	return 0;
}

