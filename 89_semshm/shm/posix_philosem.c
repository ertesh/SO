#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "err.h"

#define PSHARED 1 /* used in sem_init() indicates that \
                     semaphore is shared between processes */

const char shared_name[] = "/philosophers";

void do_nothing(int sig) { }

void block_int(void) {
  sigset_t block_mask;

  sigemptyset (&block_mask);
  sigaddset(&block_mask, SIGINT);
  if (sigprocmask(SIG_BLOCK, &block_mask, 0) == -1) syserr("sigprocmask");
}

void register_void_int_handler(void) {
   struct sigaction setup_action;
   sigset_t block_mask;

   sigemptyset (&block_mask);
   setup_action.sa_handler = do_nothing;
   setup_action.sa_mask = block_mask;
   setup_action.sa_flags = 0;
   if (sigaction (SIGINT, &setup_action, 0) == -1)
      syserr("sigaction");

   sigaddset(&block_mask, SIGINT);
   if (sigprocmask(SIG_UNBLOCK, &block_mask, 0) == -1)
      syserr("sigprocmask");
}

void* get_posix_memory(size_t size) {
   int shmid;
   void *mem_ptr;

   if ((shmid = shm_open(shared_name, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1)
      syserr("shm_open");
   if (ftruncate(shmid, size) == -1) syserr("ftruncate");
   if ((mem_ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0))
         == (void*) -1) syserr("mmap");
   if (shm_unlink(shared_name) == -1) syserr("shm_unlink");

   return mem_ptr;
}

void* get_sysv_memory(size_t size) {
   int shmid;
   void *mem_ptr;

   if((shmid = shmget(IPC_PRIVATE, size, 0600 | IPC_CREAT | IPC_EXCL)) == -1)
      syserr("shmget error");
   if((mem_ptr = shmat(shmid, 0, 0)) == (void*)-1) syserr("shmat error");
   if(shmctl(shmid, IPC_RMID, 0) == -1) syserr("shmctl");

   return mem_ptr;
}

struct shm_struct {
   sem_t mutex, spoon[5];
} *shm_ptr;

void do_some_philosophy(int id) {
   while (1) {
      sleep(1+random()%3);

      if (sem_wait(&shm_ptr->mutex) == -1) syserr("sem_wait");
      if (sem_wait(shm_ptr->spoon + id) == -1) syserr("sem_wait");
      if (sem_wait(shm_ptr->spoon + ((id+1) % 5)) == -1) syserr("sem_wait");

      printf("philo #%d: eat, ate, eaten ;)\n", id);
      fflush(NULL);
      sleep(1);

      if (sem_post(&shm_ptr->mutex) == -1) syserr("sem_post");
      if (sem_post(shm_ptr->spoon + id) == -1) syserr("sem_post");
      if (sem_post(shm_ptr->spoon + ((id+1) % 5)) == -1) syserr("sem_post");
   }
}

int main (void) {
   int id, pid[5];

   /* Both calls below comply with standard but            *
    * the POSIX open_shm call has not been implemented yet */
/**///   shm_ptr = (struct shm_struct*)get_posix_memory(sizeof(struct shm_struct));
   shm_ptr = (struct shm_struct*)get_sysv_memory(sizeof(struct shm_struct));

   if (sem_init(&shm_ptr->mutex, PSHARED, 4) == -1) syserr("sem_init");
   for (id = 0; id < 5; ++id)
      if (sem_init(&shm_ptr->spoon[id], PSHARED, 1) == -1) syserr("sem_init");

   for (id = 0; id < 5; ++id)
      switch (pid[id] = fork()) {
         case -1  :
            syserr("fork");
         case 0   :
            do_some_philosophy(id);
            return 0;
         default  :
            break;
      }

   fprintf(stderr, "Wait and observe; send ctrl-C to stop this application\n");
   pause();
   for (id = 0; id < 5; ++id)
      if (kill (pid[id], SIGINT) == -1) syserr("kill");
   for (id = 0; id < 5; ++id)
      if (wait(0) == -1) syserr("wait");
      
   if (sem_destroy(&shm_ptr->mutex) == -1) syserr("sem_destroy");
   for (id = 0; id < 5; ++id)
      if (sem_destroy(&shm_ptr->spoon[id]) == -1) syserr("sem_destroy");

   return 0;
}
