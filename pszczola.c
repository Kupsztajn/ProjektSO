#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"
#include "sem.h"

#define VISITS 5

// Indeksy semaforów w tablicy
#define SEM_ENT1 0
//#define SEM_POM 5
#define SEM_ENT2 1 
#define SEM_ULE  2 
#define SEM_POP  3 
#define SEM_KROL 4
#define SEM_LOCK 5

void bee_logic(int semid, int* P, int* shm, int* nadmiarULE, int* nadmiarPOP);

int main()
{
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(shm_key, sizeof(struct SharedMemory), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    struct SharedMemory* shm = (struct SharedMemory*)attach_shared_memory(shmid);
    if (!shm) {
        perror("attach_shared_memory failed");
        exit(EXIT_FAILURE);
    }
    shm = (struct SharedMemory*)attach_shared_memory(shmid);

    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    int semid = semget(sem_key, 6, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    bee_logic(semid, &shm->P, &shm->N, &shm->nadmiar_ULE, &shm->nadmiar_POP);

    
	return 0;
}

int release_semaphore(int semid, int sem_num) {
    struct sembuf op = { sem_num, 1, 0 };
    if (semop(semid, &op, 1) == -1) {
        perror("Failed to release semaphore");
        return -1;
    }
    return 0;
}

int acquire_semaphore(int semid, int sem_num) {
    struct sembuf op = { sem_num, -1, 0 };
    if (semop(semid, &op, 1) == -1) {
        perror("Failed to acquire semaphore");
        return -1;
    }
    return 0;
}

void release_entrance(int semid, int sem_entrance) {
    struct sembuf op_increase_entrance = { sem_entrance, 1, 0 };
    if (semop(semid, &op_increase_entrance, 1) == -1) {
        perror("Failed to release entrance semafor");
    }
}