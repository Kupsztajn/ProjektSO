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

	shm->P = 10; // Maksymalna liczba pszcz� w ulu
	shm->N = 20; // Maksymalna liczba pszcz� w populacji

	printf("Segment pami�ci wsp�dzielonej utworzony: shmid = %d\n", shmid);
	printf("Warto�ci w pami�ci wsp�dzielonej: P = %d, N = %d\n", shm->P, shm->N);

    printf("Segment pami�ci wsp�dzielonej utworzony: shmid = %d\n", shmid);
    printf("Warto�ci w pami�ci wsp�dzielonej: P = %d, N = %d\n", shm->P, shm->N);

    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    // Tworzenie tablicy semafor�w (5 semafory: SEM_ULE, SEM_POP, SEM_ENT1, SEM_ENT2)
    int semid = semget(sem_key, 6, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja warto�ci semafor�w
    inicjalizujSemafor(semid, SEM_ULE, shm->P);   // Liczba miejsc w ulu
    inicjalizujSemafor(semid, SEM_POP, shm->N);   // Ca�kowita populacja pszcz�
    inicjalizujSemafor(semid, SEM_ENT1, 1);       // Dost�pne pierwsze wej�cie/wyj�cie
    inicjalizujSemafor(semid, SEM_ENT2, 1);
    


    // Proces inicjalizuj�cy mo�e dzia�a� w p�tli, aby umo�liwi� innym procesom korzystanie z pami�ci
    while (1) sleep(1);

    // Od��czenie pami�ci wsp�dzielonej (teoretycznie nigdy tu nie dochodzi)
    detach_shared_memory(shm);

    // Usuni�cie segmentu pami�ci wsp�dzielonej
    destroy_shared_memory(shmid);

    return 0;

	return 0;
}