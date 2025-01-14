#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h" 
#include <sys/wait.h>
#include <pthread.h>

#define SEM_ENT1 0
#define SEM_ENT2 1 
#define SEM_ULE  2 
#define SEM_POP  3 
#define SEM_KROL 4
#define SEM_LOCK 5 

void queen_logic(int semid, int ilosc, int* P, int* max, int* nadmiarULE, int* nadmiarPOP);

void* zombie_collector(void* arg) {
    while (1) {
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            printf("[KROLOWA] Zebrano martwa pszczola (proces zombie).\n");
        }
        sleep(1);
    }
    return NULL;
}

int main() {
    int shmid, semid;
    struct SharedMemory* shm;

    // Wywo³anie funkcji do inicjalizacji zasobów IPC
    zbior_sem_mem(&shmid, &shm, &semid);

    printf("Pamiêæ wspó³dzielona i semafory zainicjalizowane.\n");
    printf("shmid: %d, semid: %d\n", shmid, semid);


    pthread_t zombie_thread;
    if (pthread_create(&zombie_thread, NULL, zombie_collector, NULL) != 0) {
        perror("Nie uda³o siê utworzyæ w¹tku do zbierania zombie");
        exit(EXIT_FAILURE);
    }

    printf("&shm->P: %d &shm->P %d \n", *(&shm->N), *(&shm->P));
    queen_logic(semid, *(&shm->P) - semctl(semid, SEM_ULE, GETVAL), &shm->P, &shm->N, &shm->nadmiar_ULE, &shm->nadmiar_POP);

    detach_shared_memory(shm);

    return 0;
}

void queen_logic(int semid, int ilosc, int* P, int* max, int* nadmiarULE, int* nadmiarPOP) {
    while (1) {
        printf("[Krolowa] Królowa sprawdza miejsce w ulu...Semafor_ULE: %d SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        struct sembuf wait_for_permission = { SEM_KROL, -1, 0 };
        semop(semid, &wait_for_permission, 1);

        // czy jest miejsce w ulu na nowe jaja
        int wolne_miejsca = semctl(semid, SEM_ULE, GETVAL);
        if (wolne_miejsca > 0 && semctl(semid, SEM_POP, GETVAL) > 0) {

            struct sembuf ops[] = {
            {SEM_POP, -1, 0}, // Obni¿enie semafora populacji
            {SEM_ULE, -1, 0}  // Obni¿enie semafora miejsc w ulu
            };

            // Wykonanie operacji
            if (semop(semid, ops, 2) == -1) {
                perror("semop failed for SEM_POP and SEM_ULE");
                exit(EXIT_FAILURE);
            }

            printf("[Krolowa] Miejsce dostêpne. Królowa sk³ada jajo...\n");

            printf("[Krolowa] Liczba pszczó³ w ulu po z³o¿eniu jaja: %d (maksymalnie %d)\n", *P - semctl(semid, SEM_ULE, GETVAL), *P);

            // Tworzenie nowego procesu potomnego dla pszczo³y
            pid_t pid = fork();
            if (pid == 0) {
                execl("./pszczola", "pszczola", NULL);
                perror("execl failed");
                exit(EXIT_FAILURE);
            }
            else if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
        }
        else if ((semctl(semid, SEM_POP, GETVAL) - *nadmiarPOP) < 0) {
            printf("[Krolowa] WSTRZYMANO PRODUKCJE Semafor_ULE %d SEMAFOR_POP %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        }
        else {
            printf("[Krolowa] Brak miejsca w ulu lub osiagnieto limit populacji na z³o¿enie jaj. Królowa czeka... Semafor_ULE %d SEMAFOR_POP %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
        }

        struct sembuf release_permission = { SEM_KROL, 1, 0 }; // Przywrócenie dostêpu
        semop(semid, &release_permission, 1);

        while (waitpid(-1, NULL, WNOHANG) > 0) {
            printf("[KROLOWA] Zebrano zakoñczony proces potomny (pszczo³a).\n");
        }

        //int delay = 3 + rand() % 2; //Odstep czasu skladania jaj (Tk) // Losuje wartoœæ 3, 4 lub 5
        //sleep(delay);
    }
}
