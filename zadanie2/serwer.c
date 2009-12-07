/* Maciej Andrejczuk */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>

#include "err.h"
#include "mesg.h"
#include "graph.h"


void print_usage_info(char* prog_name) {
	fatal("Correct usage:\n"
          "%s n x t\n"
          "n - number of graph nodes\n"
          "x - limit of threads\n"
          "t - maximal time that given thread should wait\n"
          "Moreover 0 <= V, W, Wi < 50 and 1 <= G < 100.\n",
			prog_name);
}

int myatoi(char* text) {
    int ret = atoi(text);
    if (ret > 0) return ret;
    if (strcmp(text, "0") == 0) return 0;
    return -1;
}

int id1 = 0, id2 = 0, id3 = 0, id4 = 0;
int is_working = 1;
Graph g;
pthread_rwlock_t* lock;
pthread_cond_t condition;
pthread_mutex_t my_mutex;

void setup_signal();

void process_signal(int nr) {
    is_working = 0;
    setup_signal();
}

void setup_signal() {
    if (signal(SIGINT, process_signal) == SIG_ERR)
        syserr("signal(SIGINT)\n");
    if (signal(SIGHUP, process_signal) == SIG_ERR)
        syserr("signal(SIGHUP)\n");
    if (signal(SIGTERM, process_signal) == SIG_ERR)
        syserr("signal(SIGTERM)\n");
}

void prepare(int n, int x) {
    int i;
    prepare_graph(&g, n);
    lock = (pthread_rwlock_t*) malloc(n * sizeof(pthread_rwlock_t));
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    for (i = 0; i < n; i++) {
        pthread_rwlock_init(lock + i, &attr);
    }
    id1 = init_queue(MKEY1, CREATE);
    id2 = init_queue(MKEY2, CREATE);
    id3 = init_queue(MKEY3, CREATE);
    id4 = init_queue(MKEY4, CREATE);
}

void process_order(int n) {
    MesgOrder message;
    MesgInfo reply;
    receive_order(id3, &message, 0);

    reply.type = message.type;
    if (message.command == ADD) {
        int x = message.data[0];
        int y = message.data[1];
        int w = message.data[2];
        if (x < 0 || x >= n || y < 0 || y >= n)
            reply.command = -1;
        else {
            /* protokol wstepny */
            int mi = (x < y) ? x : y;
            int ma = (x > y) ? x : y;
            pthread_rwlock_wrlock(lock + mi);
            pthread_rwlock_wrlock(lock + ma);
            /* sekcja krytyczna */
            reply.command = modify_graph(g, x, y, w);
            /* protokol koncowy */
            pthread_rwlock_unlock(lock + mi);
            pthread_rwlock_unlock(lock + ma);
        }
    }
    if (message.command == DEL) {
        int x = message.data[0];
        int y = message.data[1];
        if (x < 0 || x >= n || y < 0 || y >= n)
            reply.command = -1;
        else {
            /* protokol wstepny */
            int mi = (x < y) ? x : y;
            int ma = (x > y) ? x : y;
            pthread_rwlock_wrlock(lock + mi);
            pthread_rwlock_wrlock(lock + ma);
            /* sekcja krytyczna */
            reply.command = modify_graph(g, x, y, 0);
            /* protokol koncowy */
            pthread_rwlock_unlock(lock + mi);
            pthread_rwlock_unlock(lock + ma);
        }
    }
    if (message.command == FIND) {
        int i, p, czy = 1, ret;
        int ile = message.len;
        long long mask = 0;
        for (i = 0; i < ile; i++) {
            p = message.data[i];
            if (p < 0 || p >= n) {
                czy = 0;
                break;
            }
            mask |= (1 << p);
        }
        if (!czy) {
            reply.command = -1;
        }
        else {
            Graph g2;
            prepare_graph(&g2, n);
            /* protokol wstepny */
            for (i = 0; i < n; i++)
                if (mask & (1 << i)) {
                    pthread_rwlock_rdlock(lock + i);
                }
            /* sekcja krytyczna */
            copy_subgraph(&g, &g2, n, mask);
            /* protokol koncowy */
            for (i = 0; i < n; i++)
                if (mask & (1 << i)) {
                    pthread_rwlock_unlock(lock + i);
                }

            ret = find_hamiltonian(g2, n, mask);
            reply.command = ret;
        }
    }
    send_info(id2, &reply);
}

void* process(void* data) {
    int* tab= (int*) data;
    int n = (int) tab[0];
    int t = (int) tab[1];
    int numer = (int) tab[2];
    struct timespec ts;
    struct timeval tp;
    MesgInfo info;
    while (1) {
        int id = 0;
        info.type = READY;
        info.command = numer;
        send_info(id4, &info);
        gettimeofday(&tp, NULL);
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += t;
        if (pthread_mutex_lock(&my_mutex) != 0) {
            syserr("pthread_mutex_lock\n");
        }
        id = pthread_cond_timedwait(&condition, &my_mutex, &ts);
        if (id == 0) {
            if (pthread_mutex_unlock(&my_mutex) != 0) {
                syserr("pthread_mutex_unlock\n");
            }
            process_order(n);
            info.type = READY;
            send_info(id4, &info);
        } else if (id == ETIMEDOUT) {
            info.type = EXITING;
            send_info(id4, &info);
        } else {
            syserr("pthread_cond_timedwait\n");
        }
    }
    return 0;
}

void* worker(void* data) {
    MesgOrder message;
    MesgInfo info;
    while(is_working) {
        /* Receiving order from client */
        receive_order(id1, &message, 0);
        /* Forwarding order to the inner queue */
        send_order(id3, &message, (message.len + 2)*sizeof(int));
        /* Sending info about new task to the manager */
        info.type = WORK;
        info.command = 1;
        send_info(id4, &info);
        /* Waiting for a confirmation that process is executing this order */
        receive_info(id4, &info, 2);
    }
    info.type = EXITING;
    send_info(id4, &info);
    return 0;
}

void init_thread_attr(pthread_attr_t* attr) {
    int err;
    if ((err = pthread_attr_init(attr)) != 0 )
            syserr("attrinit");
    if ((err = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE)) != 0)
              syserr("setdetach");
}

void init_thread(pthread_t* name, pthread_attr_t* attr, void*(*fun)(void*),
        void* args) {
    int err;
    if ((err = pthread_create(name, attr, fun, args)) != 0)
        syserr("create");
}

void manager(int n, int x, int t) {
    pthread_attr_t attr;
    pthread_t* threads = (pthread_t*) malloc(x * sizeof(pthread_t));
    pthread_t daemon;
    MesgInfo info;
    int process_count = 0;
    int vacant_count = 0;
    int *tab = (int*) malloc(x * sizeof(int));
    void ** status = 0;
    int i;

    init_thread_attr(&attr);
    init_thread(&daemon, &attr, worker, 0);

    for (i = 0; i < x; i++) {
        tab[x] = 0;
    }
    while (1) {
        receive_info(id4, &info, 0);
        if (info.type == WORK) {
            if (vacant_count > 0) {
                /* Signal */
                vacant_count--;
            } else if (process_count < n) {
                for (i = 0; i < x; i++)
                    if (tab[i] == 0) {
                        int args[] ={n, t, i};
                        tab[i] = 1;
                        init_thread(threads + i, &attr, process, args);
                        break;
                    }
                process_count++;
            } else {
               /* Czekaj */
            }
        }
        if (info.type == READY) {
            vacant_count++;
        }
        if (info.type == FINISHED) {
            int ret;
            process_count--;
            vacant_count--;
            tab[info.command] = 0;
            ret = pthread_join(threads[info.command], status);
            /* Errory */
        }
        if (info.type == EXITING) {
            int ret = pthread_join(daemon, status);
            for (i = 0; i < x; i++) {
                if (tab[i] != 0) {
                    ret = pthread_join(threads[i], status);
                   /* Errory */
                }
            }
            break;
        }
    }
}

void clean() {

    close_queue(id1);
    close_queue(id2);
    close_queue(id3);
    close_queue(id4);

    sleep(1);
    exit(0);
}

int main(int argc, char* argv[]) {
    int n, x, t;
    if (argc != 4)
        print_usage_info(argv[0]);
    if ((n = myatoi(argv[1])) <= 0)
        print_usage_info(argv[0]);
    if ((x = myatoi(argv[2])) <= 0)
        print_usage_info(argv[0]);
    if ((t = myatoi(argv[3])) < 0)
        print_usage_info(argv[0]);
    prepare(n, x);
    manager(n, x, t);
    clean();
    return 0;
}

