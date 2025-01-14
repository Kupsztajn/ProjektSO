#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"
#include <sys/wait.h>


#define SEM_ENT1 0
#define SEM_ENT2 1 
#define SEM_ULE  2 
#define SEM_POP  3 
#define SEM_KROL 4
#define SEM_LOCK 5 

struct SharedMemory* shm;

int semid;
int shmid;
// Handler dla SIGHUP (zwiekszanie N)
void handle_sighup(int sig) {
    printf("\t \t [PSZCZELARZ] Otrzymano SIGHUP: Zmniejszanie N w pamiêci dzielonej i semaforze...\n");
    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));

    // zablokowanie semaforow wejsc, wstrzymanie produkcji jaj, oraz zajecie pamieci dzielonej
    struct sembuf lock_all[] = {
        {SEM_ENT1, -1, 0},
        {SEM_ENT2, -1, 0},
        {SEM_KROL, -1, 0},
        {SEM_LOCK, -1, 0}
    };

    semop(semid, lock_all, 3);

    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));

    if (shm->N < 100) {
        int wartN, wartP;

        wartN = shm->N * 2 - shm->N + semctl(semid, SEM_POP, GETVAL);
        shm->N *= 2;

        wartP = shm->P * 2 - shm->P + semctl(semid, SEM_ULE, GETVAL);
        shm->P *= 2;

        printf("\t \t [PSZCZELARZ] Nowa wartoœæ N: %d\n", shm->N);
        printf("\t \t [PSZCZELARZ] Nowa wartoœæ P: %d\n", shm->P);

        // Zwiekszenie wartosci semafora SEM_POP
        if (wartN > 0) {
            shm->nadmiar_POP -= abs(wartN);
            if (shm->nadmiar_POP < 0) shm->nadmiar_POP = 0;
        }
        if (semctl(semid, SEM_POP, SETVAL, wartN) == -1) {
            perror("\t \t [PSZCZELARZ] Nie uda³o siê zaktualizowaæ SEM_POP");
        }
        else {
            printf("\t \t [PSZCZELARZ] SEM_POP zaktualizowany do wartoœci: %d\n", wartN);
        }

        // Zwiekszenie wartosci semafora SEM_ULE
        if (wartP > 0) {
            shm->nadmiar_ULE -= abs(wartP);
            if (shm->nadmiar_ULE < 0) shm->nadmiar_ULE = 0;
        }
        if (semctl(semid, SEM_ULE, SETVAL, wartP) == -1) {
            perror("\t \t [PSZCZELARZ] Nie uda³o siê zaktualizowaæ SEM_ULE");
        }
        else {
            printf("\t \t [PSZCZELARZ] SEM_ULE zaktualizowany do wartoœci: %d\n", wartP);
        }

    }
    else {
        printf("\t \t [PSZCZELARZ] N jest ju¿ maksymalne. Nie mo¿na zwiekszyc dalej.\n");
    }

    printf("\t \t [PSZCZELARZ] NADMIAR_ULE: %d, NADMIAR_POP: %d \n", shm->nadmiar_ULE, shm->nadmiar_POP);


    // Odblokowanie semaforow wejsc, wznowienie produkcji jaj, oraz odblokowanie pamieci dzielonej
    struct sembuf unlock_all[] = {
        {SEM_ENT1, 1, 0},
        {SEM_ENT2, 1, 0},
        {SEM_KROL, 1, 0},
        {SEM_LOCK, 1, 0}
    };

    semop(semid, unlock_all, 3);
}

// Handler dla SIGQUIT (zmniejszanie N)
void handle_sigquit(int sig) {
    printf("\t \t [PSZCZELARZ] Otrzymano SIGQUIT: Zmniejszanie N w pamiêci dzielonej i semaforze...\n");
    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));

    // zablokowanie semaforow wejsc, wstrzymanie produkcji jaj, oraz zajecie pamieci dzielonej
    struct sembuf lock_all[] = {
        {SEM_ENT1, -1, 0},
        {SEM_ENT2, -1, 0},
        {SEM_KROL, -1, 0},
        {SEM_LOCK, -1, 0}
    };
    semop(semid, lock_all, 3);

    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
    if (shm->N > 2) {
        int wartN, wartP;

        wartN = shm->N / 2 - shm->N + semctl(semid, SEM_POP, GETVAL);
        shm->N /= 2;

        wartP = shm->P / 2 - shm->P + semctl(semid, SEM_ULE, GETVAL);
        shm->P /= 2;

        printf("\t \t [PSZCZELARZ] Nowa wartoœæ N: %d\n", shm->N);
        printf("\t \t [PSZCZELARZ] Nowa wartoœæ P: %d\n", shm->P);

        // Zmniejszenie wartosci semafora SEM_POP
        if (wartN < 0) {
            shm->nadmiar_POP += abs(wartN);
            wartN = 0;
        }
        if (semctl(semid, SEM_POP, SETVAL, wartN) == -1) {
            perror("\t \t [PSZCZELARZ] Nie uda³o siê zaktualizowaæ SEM_POP");
        }
        else {
            printf("\t \t [PSZCZELARZ] SEM_POP zaktualizowany do wartoœci: %d\n", wartN);
        }

        // Zmniejszenie wartosci semafora SEM_ULE
        if (wartP < 0) {
            shm->nadmiar_ULE += abs(wartP);
            wartP = 0;
        }
        if (semctl(semid, SEM_ULE, SETVAL, wartP) == -1) {
            perror("\t \t [PSZCZELARZ] Nie uda³o siê zaktualizowaæ SEM_ULE");
        }
        else {
            printf("\t \t [PSZCZELARZ] SEM_ULE zaktualizowany do wartoœci: %d\n", wartP);
        }

    }
    else {
        printf("\t \t [PSZCZELARZ] N jest ju¿ minimalne. Nie mo¿na zmniejszyæ dalej.\n");
    }
    printf("\t \t [PSZCZELARZ] NADMIAR_ULE: %d, NADMIAR_POP: %d \n", shm->nadmiar_ULE, shm->nadmiar_POP);

    // Odblokowanie semaforow wejsc, wznowienie produkcji jaj, oraz odblokowanie pamieci dzielonej
    struct sembuf unlock_all[] = {
        {SEM_ENT1, 1, 0},
        {SEM_ENT2, 1, 0},
        {SEM_KROL, 1, 0},
        {SEM_LOCK, 1, 0}
    };

    semop(semid, unlock_all, 3);
}


// Funkcja rejestrujaca handler sygna³ow za pomoca sigaction
void setup_signal_handler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(signal, &sa, NULL) == -1) {
        perror("\t \t [PSZCZELARZ] Nie uda³o siê zarejestrowaæ handlera sygna³u");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Wywolanie funkcji do inicjalizacji zasobow IPC
    zbior_sem_mem(&shmid, &shm, &semid);

    setup_signal_handler(SIGHUP, handle_sighup);
    setup_signal_handler(SIGQUIT, handle_sigquit);

    printf("\t \t [PSZCZELARZ] Pszczelarz uruchomiony. PID: %d\n", getpid());
    printf("\t \t [PSZCZELARZ] Oczekiwanie na sygna³y SIGHUP i SIGQUIT...\n");


    while (1) {
        pause();
    }

    detach_shared_memory(shm);

    return 0;
}
