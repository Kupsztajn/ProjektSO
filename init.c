#include "shm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#define SEM_ENT1 0
#define SEM_ENT2 1 
#define SEM_ULE  2 
#define SEM_POP  3 
#define SEM_KROL 4
#define SEM_LOCK 5 


int main() {
    int shmid, semid;
    struct SharedMemory* shm;

    // Wywolanie funkcji do inicjalizacji zasobów IPC
    zbior_sem_mem(&shmid, &shm, &semid);

    shm->nadmiar_ULE = 0;
    shm->nadmiar_POP = 0;

    printf("Segment pamiêci wspó³dzielonej utworzony: shmid = %d\n", shmid);
    printf("Wartoœci w pamiêci wspó³dzielonej: P = %d, N = %d\n", shm->P, shm->N);

    init_semaphore(semid, SEM_ULE, shm->P);
    init_semaphore(semid, SEM_POP, shm->N);
    init_semaphore(semid, SEM_ENT1, 1);
    init_semaphore(semid, SEM_ENT2, 1);
    init_semaphore(semid, SEM_KROL, 1);
    init_semaphore(semid, SEM_LOCK, 1);

    while (1) sleep(1);

    detach_shared_memory(shm);

    return 0;
}