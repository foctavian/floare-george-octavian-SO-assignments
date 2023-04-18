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

int main() {

    pid_t pid2, pid3, pid4, pid5, pid6, pid7;

    // printf("%d\n", getpid());

    init();
    info(BEGIN, 1, 0);
    pid2 = fork();

    if (pid2 == 0) {
        info(BEGIN, 2, 0);

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
            //P5 as child
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
