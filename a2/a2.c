#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <signal.h>
#include <unistd.h>
#include "a2_helper.h"

int _t1_start_flag = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done_condition = PTHREAD_COND_INITIALIZER;



typedef struct {
    int pid;
    int tid;
    pthread_mutex_t lock;
} TH_INFO;

//TODO: double conditional mutex to end t1 after t3

void* function(void* param) {
    TH_INFO* s = (TH_INFO*)param;
    if (s->tid == 3) {
        pthread_mutex_lock(&lock);
        while (!_t1_start_flag) {
            pthread_cond_wait(&done_condition, &lock);
        }
        pthread_mutex_unlock(&lock);
    }
    info(BEGIN, s->pid, s->tid);
    if (s->tid == 1) {
        pthread_mutex_lock(&lock);
        _t1_start_flag = 1;
        pthread_cond_signal(&done_condition);
        pthread_mutex_unlock(&lock);
    }
    info(END, s->pid, s->tid);
    return NULL;
}

int main() {

    pid_t pid2, pid3, pid4, pid5, pid6, pid7;
    pthread_t tid[4] = { -1, -1, -1, -1};
    pthread_mutex_t lock;
    init();

    info(BEGIN, 1, 0);
    pid2 = fork();

    if (pid2 == 0) {
        info(BEGIN, 2, 0);
        //2.3
        TH_INFO th_info[4];
        if (pthread_mutex_init(&lock, NULL) != 0) {
            return -1;
        }
        for (int i = 0; i < 4; i++) {
            th_info[i].tid = i + 1;
            th_info[i].pid = 2;
            th_info[i].lock = lock;
        }

        for (int i = 1; i <= 4; i++) {
            if (pthread_create(&tid[i - 1], NULL, function, &th_info[i - 1]) != 0) {
                perror("Error creating thread");
                return -1;
            }
        }

        for (int i = 0; i < 4; i++) {
            if (i == 0) {
                pthread_join(tid[2], NULL);
            }
        }
        pthread_mutex_destroy(&lock);

        pid3 = fork();
        if (pid3 == 0) {
            info(BEGIN, 3, 0);
            info(END, 3, 0);
            exit(0);
        } else {
            wait(NULL);
        }
        pid4 = fork();
        if (pid4 == 0) {
            info(BEGIN, 4, 0);
            info(END, 4, 0);
            exit(0);
        }
        else {
            wait(NULL);
        }
        pid5 = fork();
        if (pid5 == 0) {
            info(BEGIN, 5, 0);
            pid6 = fork();
            if (pid6 == 0) {
                info(BEGIN, 6, 0);
                info(END, 6, 0);
                exit(0);
            } else {
                pid7 = fork();
                if (pid7 == 0) {
                    info(BEGIN, 7, 0);
                    info(END, 7, 0);
                    exit(0);
                } else {
                    wait(NULL);
                }
            }
            info(END, 5, 0);
            exit(0);
        } else {
            wait(NULL);
        }
        info(END, 2, 0);
        exit(0);
    }
    else {
        wait(NULL);
    }

    info(END, 1, 0);
    exit(0);
    return 0;
}
