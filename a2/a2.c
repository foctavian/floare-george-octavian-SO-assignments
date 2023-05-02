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

//TODO: t2 trebuie sa inceapa inainte de t3 si sa se termine dupa t3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condT2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t condT3 = PTHREAD_COND_INITIALIZER;


void* function(void* param) {
    TH_INFO* s = (TH_INFO*)param;
    info(BEGIN, s->pid, s->tid);
    info(END, s->pid, s->tid);
    return NULL;
}


int main() {

    pid_t pid2, pid3, pid4, pid5, pid6, pid7;
    pthread_t tid[4];
    for (int i = 0; i < 4; i++) {
        tid[i] = -1;
    }

    init();

    info(BEGIN, 1, 0);
    pid2 = fork();

    if (pid2 == 0) {
        info(BEGIN, 2, 0);
        //2.3
        TH_INFO th_info[4];
        sem_t lock;
        if (sem_init(&lock, 0, 1) != 0) {
            perror("Could not init the semaphore");
            return -1;
        }
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
            if(i == 1){
                pthread_join(tid[1], NULL);
            }
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
