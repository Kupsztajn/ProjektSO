#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"

#define VISITS 50

// indeksy semforow
#define SEM_ENT1 0
#define SEM_ENT2 1 
#define SEM_ULE  2 
#define SEM_POP  3 
#define SEM_KROL 4
#define SEM_LOCK 5 

void bee_logic(int semid, int* P, int* shm, int* nadmiarULE, int* nadmiarPOP);

int main() {

    int shmid, semid;
    struct SharedMemory* shm;

    // Wywolanie funkcji do inicjalizacji zasobow IPC
    zbior_sem_mem(&shmid, &shm, &semid);

    // Symulacja pracy pszczoly
    bee_logic(semid, &shm->P, &shm->N, &shm->nadmiar_ULE, &shm->nadmiar_POP);

    // Odlaczenie pamieci wspoldzielonej
    detach_shared_memory(shm);
}

void bee_logic(int semid, int* P, int* N, int* nadmiarULE, int* nadmiarPOP) {
    int odwiedziny = VISITS;
    int czy_zwiekszyc_ule = 1;
    int czy_zwiekszyc_pop = 1;
    while (odwiedziny--) {

        //sleep(rand() % 2 + 3); // Pszczola w ulu

        //int delay = 100 + rand() % 401;
        //usleep(delay);

        // Wybor wejscia przy wyjsciu
        int entrance = rand() % 2;
        int sem_entrance = (entrance == 0) ? SEM_ENT1 : SEM_ENT2;

        // Atomowe zajecie wejscia i zwiekszenie SEM_ULE
        acquire_semaphore(semid, SEM_LOCK);
        if (*nadmiarULE > 0 || semctl(semid, SEM_ULE, GETVAL) > *P) {
            czy_zwiekszyc_ule = 0;
        }
        else {
            czy_zwiekszyc_ule = 1;
        }

        // Zajecie wylosowanego wejscia i potencjalne zwiekszenie SEM_ULE jezeli nie ma nadmiaru
        struct sembuf exit_ops[] = {
            {sem_entrance, -1, 0},
            {SEM_ULE, czy_zwiekszyc_ule, 0}
        };


        if (semop(semid, exit_ops, 2) == -1) {
            perror("semop failed during exit");
            continue;
        }

        if (*nadmiarULE > 0) (*nadmiarULE)--;
        else *nadmiarULE = 0;

        printf("\t [Pszczola] Pszczo³a opuszcza ul przez wejœcie %d. \n", entrance + 1);
        release_semaphore(semid, SEM_LOCK);
        release_semaphore(semid, entrance);

        // Symulacja pracy na zewn¹trz
        //sleep(rand() % 2 + 5);
        //delay = 100 + rand() % 401;
        //usleep(delay);

        // Wybór wejœcia przy powrocie
        entrance = rand() % 2;
        sem_entrance = (entrance == 0) ? SEM_ENT1 : SEM_ENT2;

        // Atomowe zajêcie wejscia i zmniejszenie SEM_ULE
        struct sembuf enter_ops[] = {
            {sem_entrance, -1, 0},
            {SEM_ULE, -1, 0}
        };

        if (semop(semid, enter_ops, 2) == -1) {
            perror("semop failed during exit");
            continue;
        }

        printf("\t [Pszczola] Pszczo³a wesz³a do ula przez wejœcie %d. \n", entrance + 1);

        // Zwolnij wejœcie
        release_semaphore(semid, sem_entrance);

    }

    // Zajecie semafora lock
    acquire_semaphore(semid, SEM_LOCK);

    // Sprawdzanie czy jest nadmiar pszczol
    if (*nadmiarPOP > 0) {
        czy_zwiekszyc_pop = 0;
    }
    else {
        czy_zwiekszyc_pop = 1;
    }

    // Zwolnienie semafora lock
    release_semaphore(semid, SEM_LOCK);

    struct sembuf death[] = {
        //{SEM_POP, 1, 0}, // podniesienie semafora poopulacji
        //{SEM_ULE, 1, 0}        // Zwolnienie miejsca w ulu
        {SEM_POP, czy_zwiekszyc_pop, 0},
        {SEM_ULE, 1, 0}
    };

    if (semop(semid, death, 2) == -1) {
        perror("semop failed during exit");
    }

    czy_zwiekszyc_pop = 1;

    printf("\t [Pszczola] Pszczola umarla Semafor_ULE: %d \n", semctl(semid, SEM_ULE, GETVAL));

    // Zajecie semafora lock do pamieci dzielonej
    acquire_semaphore(semid, SEM_LOCK);

    if (*nadmiarULE > 0) {
        printf("\t [Pszczola] NADMIAR PSZCZOL, PSZCZOLA UMIERA , BRAK DLA NIEJ MIEJSCA");
        if (*nadmiarULE > 0) { (*nadmiarULE)--; (*nadmiarPOP)--; }
    }

    if (*nadmiarPOP > 0) (*nadmiarPOP)--;

    // Zwolnienie semafora lock do pamieci dzielonej
    release_semaphore(semid, SEM_LOCK);

    exit(0);
}
