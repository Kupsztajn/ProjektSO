#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"
#include "sem.h" 
#include <sys/wait.h>
#include <pthread.h> // Dodaj nag��wek dla w�tk�w

#define SEM_ULE 2 
#define SEM_POP 3
#define SEM_ENT1 0
#define SEM_ENT2 1
#define SEM_KROL 4
#define SEM_LOCK 5

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"

// Funkcja do inicjalizacji pami�ci wsp�dzielonej i semafor�w
/*
void zbior_sem_mem(int* shmid, struct SharedMemory** shm, int* semid) {
    // Klucz do pami�ci wsp�dzielonej
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja pami�ci wsp�dzielonej
    *shmid = shmget(shm_key, sizeof(struct SharedMemory), IPC_CREAT | 0600);
    if (*shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    *shm = (struct SharedMemory*) attach_shared_memory(*shmid);
    if (*shm == NULL) {
        perror("attach_shared_memory failed");
        exit(EXIT_FAILURE);
    }

    // Klucz do semafor�w
    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semafor�w
    *semid = semget(sem_key, 6, IPC_CREAT | 0600);
    if (*semid == -1) {
        perror("semget failed");
        detach_shared_memory(*shm);
        exit(EXIT_FAILURE);
    }
}
*/

void queen_logic(int semid, int* ilosc, int* P, int* max, int* nadmiarULE, int* nadmiarPOP);

void* zombie_collector(void* arg) {
    while (1) {
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            printf("[KROLOWA] Zebrano zako�czony proces potomny (pszczo�a).\n");
        }
        sleep(1);
    }
    return NULL;
}

int main() {
    /*
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    //pami�c wsp�dzielona
    int shmid = shmget(shm_key, sizeof(struct SharedMemory), IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    struct SharedMemory* shm = (struct SharedMemory*) attach_shared_memory(shmid);

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
    */
    /*
    // Inicjalizacja warto�ci semafor�w
    inicjalizujSemafor(semid, SEM_ULE, shm->P);   // Liczba miejsc w ulu
    inicjalizujSemafor(semid, SEM_POP, shm->N);   // Ca�kowita populacja pszcz�
    inicjalizujSemafor(semid, SEM_ENT1, 1);       // Dost�pne pierwsze wej�cie/wyj�cie
    inicjalizujSemafor(semid, SEM_ENT2, 1);       // Dost�pne drugie wej�cie/wyj�cie

    */
    int shmid, semid;
    struct SharedMemory* shm;

    // Wywo�anie funkcji do inicjalizacji zasob�w IPC
    zbior_sem_mem(&shmid, &shm, &semid);

    // Teraz mo�esz u�ywa� `shm`, `shmid` i `semid`
    printf("Pami�� wsp�dzielona i semafory zainicjalizowane.\n");
    printf("shmid: %d, semid: %d\n", shmid, semid);


    pthread_t zombie_thread;
    if (pthread_create(&zombie_thread, NULL, zombie_collector, NULL) != 0) {
        perror("Nie uda�o si� utworzy� w�tku do zbierania zombie");
        exit(EXIT_FAILURE);
    }

    printf("&shm->P: %d &shm->P %d \n", *(&shm->N), *(&shm->P));
    queen_logic(semid, *(&shm->P) - semctl(semid, SEM_ULE, GETVAL), &shm->P, &shm->N, &shm->nadmiar_ULE, &shm->nadmiar_POP);

    detach_shared_memory(shm);

    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Failed to remove semaphore array");
    }

    return 0;
}

void queen_logic(int semid, int* ilosc, int* P, int* max, int* nadmiarULE, int* nadmiarPOP) {
    while (1) {
        printf("[Krolowa] Kr�lowa sprawdza miejsce w ulu...Semafor_ULE: %d SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        struct sembuf wait_for_permission = { SEM_KROL, -1, 0 };
        semop(semid, &wait_for_permission, 1);

        //struct sembuf lock_counters = {SEM_LOCK, -1, 0};
        //semop(semid, &lock_counters, 1);
        //acquire_semaphore(semid, SEM_LOCK);
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
                perror("execl failed");
                exit(EXIT_FAILURE);
            }
            else if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
        }
        else if ((semctl(semid, SEM_POP, GETVAL) - *nadmiarPOP) < 0)
        {
            printf("[Krolowa] WSTRZYMANO PRODUKCJE Semafor_ULE %d SEMAFOR_POP %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        }
        else {
            printf("[Krolowa] Brak miejsca w ulu lub osiagnieto limit populacji na z�o�enie jaj. Kr�lowa czeka... Semafor_ULE %d SEMAFOR_POP %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        }
        //Czas sk�adania jaj (Tk)
        //release_semaphore(semid, SEM_LOCK);

        //struct sembuf unlock_counters = {SEM_LOCK, 1, 0};
        //semop(semid, &unlock_counters, 1);

        struct sembuf release_permission = { SEM_KROL, 1, 0 }; // Przywr�cenie dost�pu
        semop(semid, &release_permission, 1);

        while (waitpid(-1, NULL, WNOHANG) > 0) {
            printf("[KROLOWA] Zebrano zako�czony proces potomny (pszczo�a).\n");
        }

        int delay = 0 + rand() % 2; // Losuje warto�� 3, 4 lub 5
        //sleep(delay);
    }
}
