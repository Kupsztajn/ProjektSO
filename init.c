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

int main()
{
	int shmid = create_shared_memory("/tmp", 'A', sizeof(struct SharedMemory));

	shm->P = 10; // Maksymalna liczba pszczó³ w ulu
	shm->N = 20; // Maksymalna liczba pszczó³ w populacji

	printf("Segment pamiêci wspó³dzielonej utworzony: shmid = %d\n", shmid);
	printf("Wartoœci w pamiêci wspó³dzielonej: P = %d, N = %d\n", shm->P, shm->N);

    printf("Segment pamiêci wspó³dzielonej utworzony: shmid = %d\n", shmid);
    printf("Wartoœci w pamiêci wspó³dzielonej: P = %d, N = %d\n", shm->P, shm->N);

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

    // Inicjalizacja wartoœci semaforów
    inicjalizujSemafor(semid, SEM_ULE, shm->P);   
    inicjalizujSemafor(semid, SEM_POP, shm->N);   
    inicjalizujSemafor(semid, SEM_ENT1, 1); 
    inicjalizujSemafor(semid, SEM_ENT2, 1);
    inicjalizujSemafor(semid, SEM_KROL, 1);
    inicjalizujSemafor(semid, SEM_LOCK, 1);


    
    while (1) sleep(1);

 
    detach_shared_memory(shm);


    destroy_shared_memory(shmid);

    return 0;

	return 0;
}