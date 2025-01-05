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

    // Tworzenie tablicy semaforów (5 semafory: SEM_ULE, SEM_POP, SEM_ENT1, SEM_ENT2)
    int semid = semget(sem_key, 6, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja wartoœci semaforów
    inicjalizujSemafor(semid, SEM_ULE, shm->P);   // Liczba miejsc w ulu
    inicjalizujSemafor(semid, SEM_POP, shm->N);   // Ca³kowita populacja pszczó³
    inicjalizujSemafor(semid, SEM_ENT1, 1);       // Dostêpne pierwsze wejœcie/wyjœcie
    inicjalizujSemafor(semid, SEM_ENT2, 1);
    


    // Proces inicjalizuj¹cy mo¿e dzia³aæ w pêtli, aby umo¿liwiæ innym procesom korzystanie z pamiêci
    while (1) sleep(1);

    // Od³¹czenie pamiêci wspó³dzielonej (teoretycznie nigdy tu nie dochodzi)
    detach_shared_memory(shm);

    // Usuniêcie segmentu pamiêci wspó³dzielonej
    destroy_shared_memory(shmid);

    return 0;

	return 0;
}