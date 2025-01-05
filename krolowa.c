#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"
#include "sem.h"

#define SEM_ULE 2 
#define SEM_POP 3
#define SEM_ENT1 0
#define SEM_ENT2 1
#define SEM_KROL 4
#define SEM_LOCK 5
/*#define SEM_ULE 0  // Semafor na liczb� pszcz� w ulu
#define SEM_POP 1  // Semafor na liczb� pszcz� w populacji
#define SEM_ENT1 2 // Semafor na pierwsze wej�cie/wyj�cie
#define SEM_ENT2 3 // Semafor na drugie wej�cie/wyj�cie*/

void queen_logic(int semid, int* ilosc, int* P, int* max);

int main()
{
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    //pami�c wsp�dzielona
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
     // Inicjalizacja warto�ci semafor�w
     inicjalizujSemafor(semid, SEM_ULE, shm->P);   // Liczba miejsc w ulu
     inicjalizujSemafor(semid, SEM_POP, shm->N);   // Ca�kowita populacja pszcz�
     inicjalizujSemafor(semid, SEM_ENT1, 1);       // Dost�pne pierwsze wej�cie/wyj�cie
     inicjalizujSemafor(semid, SEM_ENT2, 1);       // Dost�pne drugie wej�cie/wyj�cie

     */

    printf("&shm->P: %d &shm->P %d \n", *(&shm->N), *(&shm->P));
    queen_logic(semid, *(&shm->P) - semctl(semid, SEM_ULE, GETVAL), &shm->P, &shm->N);

    detach_shared_memory(shm);

    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Failed to remove semaphore array");
    }

	return 0;
}

void queen_logic(int semid, int* ilosc, int* P, int* max) {
    while (1) {
        printf("[Krolowa] Kr�lowa sprawdza miejsce w ulu...Semafor_ULE: %d SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        struct sembuf wait_for_permission = { SEM_KROL, -1, 0 };
        semop(semid, &wait_for_permission, 1);

        //struct sembuf lock_counters = {SEM_LOCK, -1, 0};
        //semop(semid, &lock_counters, 1);

        // czy jest miejsce w ulu na nowe jaja
        int wolne_miejsca = semctl(semid, SEM_ULE, GETVAL);
        if (wolne_miejsca > 0 && semctl(semid, SEM_POP, GETVAL) > 0) {
            // Obni�enie `SEM_POP`
            struct sembuf op_decrease_pop = { SEM_POP, -1, 0 };
            if (semop(semid, &op_decrease_pop, 1) == -1) {
                perror("semop failed for SEM_POP");
                exit(EXIT_FAILURE);
            }

            // Obni�enie `SEM_ULE` rezerwacja miejsca w ulu
            struct sembuf op_decrease_ule = { SEM_ULE, -1, 0 };
            if (semop(semid, &op_decrease_ule, 1) == -1) {
                perror("semop failed for SEM_ULE");
                exit(EXIT_FAILURE);
            }

            printf("[Krolowa] Miejsce dost�pne. Kr�lowa sk�ada jajo...\n");

            // Aktualizacja liczby pszcz� w ulu
            //*(ilosc)++;
            printf("[Krolowa] Liczba pszcz� w ulu po z�o�eniu jaja: %d (maksymalnie %d)\n", *P - semctl(semid, SEM_ULE, GETVAL), *P);

            // Tworzenie nowego procesu potomnego dla pszczo�y
            pid_t pid = fork();
            if (pid == 0) {
                // Wyb�r wej�cia/wyj�cia
                /*
                int entrance = rand() % 2;
                int sem_entrance = (entrance == 0) ? SEM_ENT1 : SEM_ENT2;

                // Zaj�cie wej�cia/wyj�cia
                struct sembuf op_decrease_entrance = {sem_entrance, -1, 0};
                if (semop(semid, &op_decrease_entrance, 1) == -1) {
                    perror("semop failed for entrance semaphore");
                    exit(EXIT_FAILURE);
                }
                */

                execl("./pszczola", "pszczola", NULL);
                perror("execl failed"); // execl si� nie powiedzie
                exit(EXIT_FAILURE);
            }
            else if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
        }
        else if (semctl(semid, SEM_POP, GETVAL) == 0)
        {
            printf("[Krolowa] WSTRZYMANO PRODUKCJE Semafor_ULE %d SEMAFOR_POP %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        }
        else {
            printf("[Krolowa] Brak miejsca w ulu na z�o�enie jaj. Kr�lowa czeka... Semafor_ULE %d SEMAFOR_POP %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        }
        // Czas sk�adania jaj (Tk)

        //struct sembuf unlock_counters = {SEM_LOCK, 1, 0};
        //semop(semid, &unlock_counters, 1);

        struct sembuf release_permission = { SEM_KROL, 1, 0 }; // Przywr�cenie dost�pu
        semop(semid, &release_permission, 1);

        sleep(5);

    }

}