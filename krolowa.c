#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"
#include "sem.h"

int main()
{
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    //pamiêc wspó³dzielona
    int shmid = shmget(shm_key, sizeof(struct SharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    struct SharedMemory* shm = (struct SharedMemory*)attach_shared_memory(shmid);

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

        /*
     // Inicjalizacja wartoœci semaforów
     inicjalizujSemafor(semid, SEM_ULE, shm->P);   // Liczba miejsc w ulu
     inicjalizujSemafor(semid, SEM_POP, shm->N);   // Ca³kowita populacja pszczó³
     inicjalizujSemafor(semid, SEM_ENT1, 1);       // Dostêpne pierwsze wejœcie/wyjœcie
     inicjalizujSemafor(semid, SEM_ENT2, 1);       // Dostêpne drugie wejœcie/wyjœcie

     */

    printf("&shm->P: %d &shm->P %d \n", *(&shm->N), *(&shm->P));
    queen_logic(semid, *(&shm->P) - semctl(semid, SEM_ULE, GETVAL), &shm->P, &shm->N);

    detach_shared_memory(shm);

    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Failed to remove semaphore array");
    }

	return 0;
}