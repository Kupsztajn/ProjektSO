#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"

#define VISITS 50

// indeksy semfaorow
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

    // Wywo³anie funkcji do inicjalizacji zasobów IPC
    zbior_sem_mem(&shmid, &shm, &semid);

    // Symulacja pracy pszczo³y
    bee_logic(semid, &shm->P, &shm->N, &shm->nadmiar_ULE, &shm->nadmiar_POP);

    // Od³¹czenie pamiêci wspó³dzielonej
    detach_shared_memory(shm);
}
/*
int release_semaphore(int semid, int sem_num) {
    struct sembuf op = {sem_num, 1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("Failed to release semaphore");
        return -1;
    }
    return 0;
}

int acquire_semaphore(int semid, int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("Failed to acquire semaphore");
        return -1;
    }
    return 0;
}

void release_entrance(int semid, int sem_entrance) {
    struct sembuf op_increase_entrance = {sem_entrance, 1, 0};
    if (semop(semid, &op_increase_entrance, 1) == -1) {
        perror("Failed to release entrance semafor");
    }
}
*/

void bee_logic(int semid, int* P, int* N, int* nadmiarULE, int* nadmiarPOP) {
    int odwiedziny = VISITS;
    int czy_zwiekszyc_ule = 1;
    int czy_zwiekszyc_pop = 1;
    while (odwiedziny--) {

        //sleep(rand() % 5 + 1); // Pszczola w ulu

        // Wybór wejœcia przy wyjœciu
        int entrance = rand() % 2;
        int sem_entrance = (entrance == 0) ? SEM_ENT1 : SEM_ENT2;

        // Atomowe zajêcie wejœcia i zwiêkszenie SEM_ULE
        acquire_semaphore(semid, SEM_LOCK);
        if (*nadmiarULE > 0 && semctl(semid, SEM_ULE, GETVAL) > *P) {
            czy_zwiekszyc_ule = 0;
        }
        else
        {
            czy_zwiekszyc_ule = 1;
        }
        //release_semaphore(semid, SEM_LOCK);
        struct sembuf exit_ops[] = {
            {sem_entrance, -1, 0}, // Zajêcie wejœcia
            {SEM_ULE, czy_zwiekszyc_ule, 0}        // Zwolnienie miejsca w ulu
        };


        if (semop(semid, exit_ops, 2) == -1) {
            perror("semop failed during exit");
            continue;
        }
        //czy_zwiekszyc = -1;
        if (*nadmiarULE > 0) (*nadmiarULE)--;
        else *nadmiarULE = 0;

        //printf("\t [Pszczola] Pszczo³a opuszcza ul przez wejœcie %d Liczba pszczol w ulu %d = %d - %d + %d.\n", entrance + 1, *P-semctl(semid, SEM_ULE, GETVAL) + *nadmiarULE, *P, semctl(semid, SEM_ULE, GETVAL), *nadmiarULE);
        printf("\t [Pszczola] Pszczo³a opuszcza ul przez wejœcie %d. \n", entrance + 1);
        release_semaphore(semid, SEM_LOCK);
        release_entrance(semid, entrance);

        // Symulacja pracy na zewn¹trz
        //sleep(rand() % 5 + 5);

        // Wybór wejœcia przy powrocie
        entrance = rand() % 2;
        sem_entrance = (entrance == 0) ? SEM_ENT1 : SEM_ENT2;

        // Zajêcie wejœcia
        //if (*nadmiarULE > 0) {
        //    czy_zwiekszyc = 0;
        //}

        struct sembuf enter_ops[] = {
            {sem_entrance, -1, 0}, // Zajêcie wejœcia
            {SEM_ULE, -1, 0}        // zajecie miejsca w ulu
        };

        //czy_zwiekszyc = -1;

        if (semop(semid, enter_ops, 2) == -1) {
            perror("semop failed during exit");
            continue;
        }


        //printf("\t [Pszczola] Pszczo³a wesz³a do ula przez wejœcie %d.Liczba pszczol %d = %d - %d + %d.\n", entrance + 1, *P-semctl(semid, SEM_ULE, GETVAL) + *nadmiarULE, *P, semctl(semid, SEM_ULE, GETVAL), *nadmiarULE);
        printf("\t [Pszczola] Pszczo³a wesz³a do ula przez wejœcie %d. \n", entrance + 1);
        // Zwolnij wejœcie
        release_semaphore(semid, sem_entrance);

    }
    /*
    if (semctl(semid, SEM_POP, GETVAL) >= *N) {
        czy_zwiekszyc_pop = 0;
    }
    else {
        czy_zwiekszyc_pop = 1;
    }
    */
    acquire_semaphore(semid, SEM_LOCK);
    if (*nadmiarPOP > 0) {
        czy_zwiekszyc_pop = 0;
    }
    else {
        czy_zwiekszyc_pop = 1;
    }
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
    acquire_semaphore(semid, SEM_LOCK);

    if (*nadmiarULE > 0) {
        printf("\t [Pszczola] NADMIAR PSZCZOL, PSZCZOLA UMIERA , BRAK DLA NIEJ MIEJSCA");
        if (*nadmiarULE > 0) { (*nadmiarULE)--; (*nadmiarPOP)--; }
    }

    if (*nadmiarPOP > 0) (*nadmiarPOP)--;

    release_semaphore(semid, SEM_LOCK);
    exit(0);
}
