#include <stdio.h>
#include "sched.h"

void mysched_init() {
	printf("Init()\n");
}

mysched_thread_t mysched_create_thread(void (*starter)(), const char *name) {
	printf("Create()\n");
	return 0;
}

mysched_thread_t mysched_self() {
	return 0;
}

char * mysched_get_name(mysched_thread_t thread) {
	return 0;
}

//ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset);
//ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset);

void mysched_go() {
	printf("Go()\n");
}

