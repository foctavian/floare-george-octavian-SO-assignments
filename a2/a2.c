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
#include <semaphore.h>
#include "a2_helper.h"


typedef struct {
    int pid;
    int tid;
} TH_INFO;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_t1 = PTHREAD_COND_INITIALIZER;

int t1_flag = 0;
void* function(void* param) {
    TH_INFO* s = (TH_INFO*)param;
    if (s->tid == 3) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        info(BEGIN, s->pid, s->tid);
        info(END, s->pid, s->tid);
        pthread_cond_signal(&cond_t1);
        pthread_mutex_unlock(&mutex);
    } else {
        info(BEGIN, s->pid, s->tid);
        if (s->tid == 1) {
            pthread_cond_signal(&cond);
            pthread_mutex_lock(&mutex);
            pthread_cond_wait(&cond_t1, &mutex);
            pthread_mutex_unlock(&mutex);
        }

        info(END, s->pid, s->tid);
    }
    return NULL;
}


int main() {

    pid_t pid2, pid3, pid4, pid5, pid6, pid7;
    pthread_t tid[4];
    for (int i = 0; i < 4; i++) {
        tid[i] = -1;
    }

    /* pthread_t tid_p5[45];
     for (int i = 0; i < 45; i++) {
         tid_p5[i] = -1;
     }*/
    init();

    info(BEGIN, 1, 0);
    pid2 = fork();

    if (pid2 == 0) {
        info(BEGIN, 2, 0);
        //2.3
        TH_INFO th_info[4];

        for (int i = 0; i < 4; i++) {
            th_info[i].tid = i + 1;
            th_info[i].pid = 2;
        }

        for (int i = 0; i < 4; i++) {
            if (pthread_create(&tid[i], NULL, function, &th_info[i]) != 0) {
                perror("Error creating thread");
                return -1;
            }
        }

        for (int i = 0; i < 4; i++) {
            pthread_join(tid[i], NULL);

        }



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

            /* TH_INFO th_info_p5[45];

             for (int i = 0; i < 45; i++) {
                 th_info_p5[i].tid = i + 1;
                 th_info_p5[i].pid = 5;
             }

             for (int i = 0; i < 45; i++) {
                 if (pthread_create(&tid_p5[i], NULL, function, &th_info_p5[i]) != 0) {
                     perror("Error creating thread");
                     return -1;
                 }
             }

             for (int i = 0; i < 45; i++) {
                pthread_join(tid_p5[i], NULL);
             }*/

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
