#include "shm.h"
#include "sem.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#define SEM_ULE 2 
#define SEM_POP 3 
#define SEM_ENT1 0 
#define SEM_ENT2 1 
#define SEM_KROL 4
#define SEM_LOCK 5

int main() {

    int shmid = create_shared_memory("/tmp", 'A', sizeof(struct SharedMemory));

    struct SharedMemory* shm = (struct SharedMemory*)attach_shared_memory(shmid);

    shm->N = 16;
    shm->P = shm->N / 2;
    shm->nadmiar_ULE = 0;
    shm->nadmiar_POP = 0;

    printf("Segment pami�ci wsp�dzielonej utworzony: shmid = %d\n", shmid);
    printf("Warto�ci w pami�ci wsp�dzielonej: P = %d, N = %d\n", shm->P, shm->N);

    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    int semid = semget(sem_key, 6, IPC_CREAT | 0600);
    if (semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    init_semaphore(semid, SEM_ULE, shm->P);
    init_semaphore(semid, SEM_POP, shm->N);
    init_semaphore(semid, SEM_ENT1, 1);
    init_semaphore(semid, SEM_ENT2, 1);
    init_semaphore(semid, SEM_KROL, 1);
    init_semaphore(semid, SEM_LOCK, 1);

    while (1) sleep(1);
    /*
    zwolnijSemafor(semid, 0);
    zwolnijSemafor(semid, 1);
    zwolnijSemafor(semid, 2);
    zwolnijSemafor(semid, 3);
    zwolnijSemafor(semid, 4);
    zwolnijSemafor(semid, 5);
    */
    detach_shared_memory(shm);

    //destroy_shared_memory(shmid);

    return 0;
}