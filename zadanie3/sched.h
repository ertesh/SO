#ifndef MYSCHED_H_234265
#define MYSCHED_H_234265

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MYSCHED_STACK_SIZE 1024*1024*1

typedef int mysched_thread_t;

void mysched_init();

mysched_thread_t mysched_create_thread(void (*starter)(), const char *name);

mysched_thread_t mysched_self();

char * mysched_get_name(mysched_thread_t thread);

//ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset);
//ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset);

void mysched_go();

#ifdef __cplusplus
}
#endif

#endif /*MYSCHED_H_234265*/
