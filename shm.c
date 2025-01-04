/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include "shm.h"
*/
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "shm.h"


struct SharedMemory {
    int P;
    int N;
};


int create_shared_memory(const char* pathname, int proj_id, size_t size) {
    key_t key = ftok(pathname, proj_id);
    if (key == -1) {
        exit(EXIT_FAILURE);
    }
    printf("Generowany klucz: %d\n", key);

    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    return shmid;
}

void detach_shared_memory(void* addr) {
    if (shmdt(addr) == -1) {
        exit(EXIT_FAILURE);
    }
}

void destroy_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        exit(EXIT_FAILURE);
    }
}