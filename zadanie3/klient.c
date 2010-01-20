#include "sched.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>


/* Test 01 */
    #define TEST1_DEPTH 35
    int test1_fibbonaci(int n)
    {
        struct timeval t;
        gettimeofday(&t, NULL);
        if (n == 0) return 0;
        if (n == 1) return 1;
        return test1_fibbonaci(n-1) + test1_fibbonaci(n - 2);
    }

    void test1_handler(int snum)
    {
        fprintf(stderr, "Test1: Zlapalem sygnal.\n");
    }

    void test1_f1()
    {
        test1_fibbonaci(TEST1_DEPTH);
    }

    int test1_slow(int n)
    {
        if (n == 0) return 7;
        if (n == 1) return 3;
        return test1_slow(n-1) + test1_slow(n - 2);
    }

    void test1_f2()
    {
        test1_slow(TEST1_DEPTH);
    }

    void test1() 
    {
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = &test1_handler;
        sigaction(SIGINT, &sa, NULL);
        mysched_create_thread(&test1_f1, "Fibbonaci");
        mysched_create_thread(&test1_f2, "Slow");
        mysched_go();
    }

/* Test 02 */
    #define TEST2_MAX_VAL 10
    int p_fd[2][2];

    void test2_dzialaj(int num)
    {
        int msg;
        char * name;
        msg = 1;
        while (msg > 0) {
            mysched_pread(p_fd[num][0], &msg, sizeof(int), 0);

            name = mysched_get_name(mysched_self());
            printf("Test2: %s: Otrzymano: %d\n", name, msg);
            fflush(stdout);
            free(name);

            if (msg > TEST2_MAX_VAL) msg = -1;
            else msg++;

            mysched_pwrite(p_fd[1-num][1], &msg, sizeof(int), 0);
            name = mysched_get_name(mysched_self());
            printf("Test2: %s: Wyslano: %d\n", name, msg);
            fflush(stdout);
            free(name);
        }
        name = mysched_get_name(mysched_self());
        printf("Test2: %s: Koncze\n", name, msg);
        fflush(stdout);
        free(name);
    }

    void test2_f1()
    {
        test2_dzialaj(0);
    }

    void test2_f2()
    {
        test2_dzialaj(1);
    }

    void test2()
    {
        int msg = 1;
        pipe(p_fd[0]);
        pipe(p_fd[1]);
        write(p_fd[1][1], &msg, sizeof(int));
        mysched_create_thread(&test2_f1, "Watek A");
        mysched_create_thread(&test2_f2, "Watek B");
        mysched_go();
        close(p_fd[0][0]); close(p_fd[0][1]);
        close(p_fd[1][0]); close(p_fd[1][1]);
    }

/* Test 3 */


    #define TEST3_IT1_COUNT 2
    #define TEST3_IT2_COUNT 4

    void test3_dzialaj(const char *fname, int it_count)
    {
        int fd;
        char buff[100];
        char * name;
        int i;
        fd = open(fname, O_RDONLY);
        for (i = 0; i < it_count; ++i) {
            buff[mysched_pread(fd, buff, sizeof(buff)-1, 0)] = '\0';
            name = mysched_get_name(mysched_self());
            fprintf(stderr, "Test3: %s: Odebralem: %s\n", name, buff);
            free(name);
        }
        close(fd);
    }

    void test3_f1()
    {
        test3_dzialaj("fifo1", TEST3_IT1_COUNT);
    }

    void test3_f2()
    {
        test3_dzialaj("fifo2", TEST3_IT2_COUNT);
    }

    void test3_handler(int snum)
    {
        fprintf(stderr, "Test3: Zlapalem sygnal.\n");
    }

    void test3()
    {
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = &test3_handler;
        sigaction(SIGINT, &sa, NULL);
        mysched_create_thread(&test3_f1, "Watek A");
        mysched_create_thread(&test3_f2, "Watek B");
        mysched_go();
    }

/* Test 04 */
    int test4_id;

    void test4_f()
    {
        test4_id = mysched_self();
    }

    void test4()
    {
        mysched_thread_t id;
        id = mysched_create_thread(&test4_f, "Watek 4");
        test4_id = 0;
        mysched_go();
        assert(test4_id == id);
    }

int main()
{
    mysched_init();
    fprintf(stderr, "Test 01\n");
    test1();
    fprintf(stderr, "Test 04\n");
    test4();
    fprintf(stderr, "Test 02\n");
    test2();
    fprintf(stderr, "Test 03\n");
    test3();
    return 0;
}
