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

int id1 = 0, id2 = 0, id3 = 0, id4 = 0, id5 = 0;
int is_working = 1;
Graph g;
pthread_rwlock_t* lock;
pthread_cond_t condition;
pthread_mutex_t my_mutex;

void setup_signal();
void clean();

void process_signal(int nr) {
    setup_signal();
    clean();
    MesgOrder message;
    message.type = 1;
    message.command = FINISH;
    send_order(id1, &message, 2 * sizeof(int));
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
    setup_signal();
    id1 = init_queue(MKEY1, CREATE);
    id2 = init_queue(MKEY2, CREATE);
    id3 = init_queue(MKEY3, CREATE);
    id4 = init_queue(MKEY4, CREATE);
    id5 = init_queue(MKEY5, CREATE);
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
            pthread_rwlock_wrlock(lock + x);
            /* sekcja krytyczna */
            reply.command = modify_graph(g, x, y, w);
            /* protokol koncowy */
            pthread_rwlock_unlock(lock + x);
        }
    }
    if (message.command == DEL) {
        int x = message.data[0];
        int y = message.data[1];
        if (x < 0 || x >= n || y < 0 || y >= n)
            reply.command = -1;
        else {
            /* protokol wstepny */
            pthread_rwlock_wrlock(lock + x);
            /* sekcja krytyczna */
            reply.command = modify_graph(g, x, y, 0);
            /* protokol koncowy */
            pthread_rwlock_unlock(lock + x);
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
    /* fprintf(stderr, "Odpowiedz: %ld %d\n",reply.type, reply.command); */
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
        int ret = 0;
        info.type = CHANGE;
        info.command = numer; /* > 0 => READY */
        send_info(id4, &info);
        gettimeofday(&tp, NULL);
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += (t > 0 ? t : 1);
        if (pthread_mutex_lock(&my_mutex) != 0) {
            syserr("pthread_mutex_lock\n");
        }
        ret = pthread_cond_timedwait(&condition, &my_mutex, &ts);
        if (pthread_mutex_unlock(&my_mutex) != 0) {
            syserr("pthread_mutex_unlock\n");
        }
        if (ret == 0) {
            process_order(n);
        } else if (ret == ETIMEDOUT) {
            info.command = -numer; /* < 0 => FINISHED */
            send_info(id4, &info);
            break;
        } else {
            syserr("pthread_cond_timedwait\n");
        }
    }
    return 0;
}

void* worker(void* data) {
    MesgOrder message;
    MesgInfo info;
    while(1) {
        /* Receiving order from client */
        receive_order(id1, &message, 0);
        if (message.command == FINISH) break;
        /* Forwarding order to the inner queue */
        send_order(id3, &message, (message.len + 2)*sizeof(int));
        /* Sending info about new task to the manager */
        info.type = NEW_WORK;
        info.command = 1;
        send_info(id4, &info);
        /* Waiting for a confirmation that process is executing this order */
        receive_info(id5, &info, 0);
    }
    info.type = CHANGE;
    info.command = EXITING;
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
    int new_work = 0;
    init_thread_attr(&attr);
    init_thread(&daemon, &attr, worker, 0);

    for (i = 0; i < x; i++) {
        tab[x] = 0;
    }
    while (1) {
        if (new_work) receive_info(id4, &info, CHANGE);
        else receive_info(id4, &info, 0);
        if (info.type == NEW_WORK) {
            new_work = 1;
        }
        else if (info.type == CHANGE) {
            if (info.command >= READY) {
                int num = info.command - 1; /* Counting from 0 */
                vacant_count++;
                tab[num] = 1;
            }
            if (info.command == EXITING) {
                if (pthread_join(daemon, status) != 0)
                        syserr("pthread_join\n");
                for (i = 0; i < x; i++) {
                    if (tab[i] != 0) {
                        if (pthread_join(threads[i], status) != 0)
                            syserr("pthread_join\n");
                    }
                }
                break;
            }
            if (info.command <= FINISHED) {
                int num = -info.command - 1; /* Counting from 0 */
                process_count--;
                vacant_count--;
                tab[num] = 0;
                if (pthread_join(threads[num], status) != 0)
                    syserr("pthread_join\n");
            }
        } else fatal("Unknown message\n");

        if (new_work) {
            if (vacant_count > 0) {
                vacant_count--;
                /*pthread_mutex_lock(&my_mutex);*/
                pthread_cond_signal(&condition);
                /*pthread_mutex_unlock(&my_mutex);*/
                new_work = 0;
                info.type = 1; /* Not really important */
                info.command = 1; /* Not really important */
                send_info(id5, &info);
            } else if (process_count < n) {
                for (i = 0; i < x; i++)
                    if (tab[i] == 0) {
                        int args[] ={n, t, i + 1};
                        tab[i] = 1;
                        init_thread(threads + i, &attr, process, args);
                        break;
                    }
                process_count++;
            }
            /* Otherwise just do nothing */
        }
    }
}

void clean() {

    close_queue(id1);
    close_queue(id2);
    close_queue(id3);
    close_queue(id4);
    close_queue(id5);
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

