#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"

#define SEM_ULE 2 
#define SEM_POP 3 
#define SEM_ENT1 0 
#define SEM_ENT2 1 
#define SEM_KROL 4
#define SEM_LOCK 5

struct SharedMemory* shm;

int semid;

void handle_sighup(int sig) {
    printf("\t \t [PSZCZELARZ] Otrzymano SIGHUP: Zmniejszanie N w pamiêci dzielonej i semaforze...\n");
    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
    // Zablokowanie semaforów wejœæ
    struct sembuf lock_all[] = {
        {SEM_ENT1, -1, 0}, // Zablokowanie SEM_ENT1
        {SEM_ENT2, -1, 0}, // Zablokowanie SEM_ENT2
        {SEM_KROL, -1, 0}, // Zablokowanie SEM_KROL
        //{SEM_POP, 0, 0},   // Blokowanie dla pewnoœci
        //{SEM_ULE, 0, 0}    // Blokowanie dla pewnoœci
    };
    semop(semid, lock_all, 3);
    //struct sembuf lock_queen = {SEM_KROL, -1, 0};
    //semop(semid, &lock_queen, 1);

    //struct sembuf lock_counters = {SEM_LOCK, -1, 0};
    //semop(semid, &lock_counters, 1);

    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
    if (shm->N < 100) {
        int wartN;
        //shm->N /= 2;
        wartN = shm->N * 2 - shm->N + semctl(semid, SEM_POP, GETVAL);
        shm->N *= 2;
        //wartN = shm->N - semctl(semid, SEM_POP, GETVAL);
        int wartP;
        //shm->N /= 2; // Zmniejszenie N o po³owê
        //shm->P = shm->N / 2;
        //wartP = semctl(semid, SEM_ULE, GETVAL) - shm->P;
        printf("wartP = %d - %d + %d \n", shm->P / 2, shm->P, semctl(semid, SEM_ULE, GETVAL));
        wartP = shm->P * 2 - shm->P + semctl(semid, SEM_ULE, GETVAL);
        shm->P *= 2;
        printf("\t \t [PSZCZELARZ] Nowa wartoœæ N: %d\n", shm->N);
        printf("\t \t [PSZCZELARZ] Nowa wartoœæ P: %d\n", shm->P);

        // Zmniejszenie wartoœci semafora SEM_POP
        if (wartN > 0) {
            shm->nadmiar_POP -= abs(wartN);
            if (shm->nadmiar_POP < 0) shm->nadmiar_POP = 0;
            //wartN = 0;
        }
        if (semctl(semid, SEM_POP, SETVAL, wartN) == -1) {
            perror("\t \t [PSZCZELARZ] Nie uda³o siê zaktualizowaæ SEM_POP");
        }
        else {
            printf("\t \t [PSZCZELARZ] SEM_POP zaktualizowany do wartoœci: %d\n", wartN);
        }

        // Zmniejszenie wartoœci semafora SEM_ULE
        if (wartP > 0) {
            shm->nadmiar_ULE -= abs(wartP);
            if (shm->nadmiar_ULE < 0) shm->nadmiar_ULE = 0;
            //wartP = 0;
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
    // Odblokowanie semaforów wejœæ

    //struct sembuf unlock_counters = {SEM_LOCK, 1, 0};
    //semop(semid, &unlock_counters, 1);

    struct sembuf unlock_all[] = {
        {SEM_ENT1, 1, 0}, // Odblokowanie SEM_ENT1
        {SEM_ENT2, 1, 0}, // Odblokowanie SEM_ENT2
        {SEM_KROL, 1, 0}  // Odblokowanie SEM_KROL
    };
    semop(semid, unlock_all, 3);
    //struct sembuf unlock_queen = {SEM_KROL, 1, 0};
    //semop(semid, &unlock_queen, 1);
}

// Handler dla SIGQUIT (zmniejszanie N)
void handle_sigquit(int sig) {
    printf("\t \t [PSZCZELARZ] Otrzymano SIGQUIT: Zmniejszanie N w pamiêci dzielonej i semaforze...\n");
    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
    // Zablokowanie semaforów wejœæ
    struct sembuf lock_all[] = {
        {SEM_ENT1, -1, 0}, // Zablokowanie SEM_ENT1
        {SEM_ENT2, -1, 0}, // Zablokowanie SEM_ENT2
        {SEM_KROL, -1, 0}, // Zablokowanie SEM_KROL
        //{SEM_POP, 0, 0},   // Blokowanie dla pewnoœci
        //{SEM_ULE, 0, 0}    // Blokowanie dla pewnoœci
    };
    semop(semid, lock_all, 3);
    //struct sembuf lock_queen = {SEM_KROL, -1, 0};
    //semop(semid, &lock_queen, 1);

    //struct sembuf lock_counters = {SEM_LOCK, -1, 0};
    //semop(semid, &lock_counters, 1);

    printf("\t \t [PSZCZELARZ] SEMAFOR_ULE: %d, SEMAFOR_POP: %d \n", semctl(semid, SEM_ULE, GETVAL), semctl(semid, SEM_POP, GETVAL));
    if (shm->N > 2) {
        int wartN;
        //shm->N /= 2;
        wartN = shm->N / 2 - shm->N + semctl(semid, SEM_POP, GETVAL);
        shm->N /= 2;
        //wartN = shm->N - semctl(semid, SEM_POP, GETVAL);
        int wartP;
        //shm->N /= 2; // Zmniejszenie N o po³owê
        //shm->P = shm->N / 2;
        //wartP = semctl(semid, SEM_ULE, GETVAL) - shm->P;
        printf("wartP = %d - %d + %d \n", shm->P / 2, shm->P, semctl(semid, SEM_ULE, GETVAL));
        wartP = shm->P / 2 - shm->P + semctl(semid, SEM_ULE, GETVAL);
        shm->P /= 2;
        printf("\t \t [PSZCZELARZ] Nowa wartoœæ N: %d\n", shm->N);
        printf("\t \t [PSZCZELARZ] Nowa wartoœæ P: %d\n", shm->P);

        // Zmniejszenie wartoœci semafora SEM_POP
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

        // Zmniejszenie wartoœci semafora SEM_ULE
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
    // Odblokowanie semaforów wejœæ

    //struct sembuf unlock_counters = {SEM_LOCK, 1, 0};
    //semop(semid, &unlock_counters, 1);

    struct sembuf unlock_all[] = {
        {SEM_ENT1, 1, 0}, // Odblokowanie SEM_ENT1
        {SEM_ENT2, 1, 0}, // Odblokowanie SEM_ENT2
        {SEM_KROL, 1, 0}  // Odblokowanie SEM_KROL
    };
    semop(semid, unlock_all, 3);
    //struct sembuf unlock_queen = {SEM_KROL, 1, 0};
    //semop(semid, &unlock_queen, 1);

}

void setup_signal_handler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;  // Ustawienie funkcji obs³ugi
    sa.sa_flags = 0;          // Brak dodatkowych flag
    sigemptyset(&sa.sa_mask); // Brak blokowanych sygna³ów podczas obs³ugi

    if (sigaction(signal, &sa, NULL) == -1) {
        perror("\t \t [PSZCZELARZ] Nie uda³o siê zarejestrowaæ handlera sygna³u");
        exit(EXIT_FAILURE);
    }
}


int main() {
    // Generowanie klucza dla pamiêci wspó³dzielonej za pomoc¹ ftok
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("\t \t [PSZCZELARZ] ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    // Uzyskanie dostêpu do segmentu pamiêci wspó³dzielonej
    int shmid = shmget(shm_key, sizeof(struct SharedMemory), 0666);
    if (shmid == -1) {
        perror("\t \t [PSZCZELARZ] shmget failed");
        exit(EXIT_FAILURE);
    }

    // Pod³¹czenie segmentu pamiêci wspó³dzielonej
    shm = (struct SharedMemory*)attach_shared_memory(shmid);

    // Generowanie klucza dla tablicy semaforów za pomoc¹ ftok
    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("\t \t [PSZCZELARZ] ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    // Uzyskanie dostêpu do tablicy semaforów
    semid = semget(sem_key, 5, 0666);
    if (semid == -1) {
        perror("\t \t [PSZCZELARZ] semget failed");
        detach_shared_memory(shm);
        exit(EXIT_FAILURE);
    }

    // Rejestracja handlerów sygna³ów za pomoc¹ sigaction

    setup_signal_handler(SIGHUP, handle_sighup);
    setup_signal_handler(SIGQUIT, handle_sigquit);

    printf("\t \t [PSZCZELARZ] Pszczelarz uruchomiony. PID: %d\n", getpid());
    printf("\t \t [PSZCZELARZ] Oczekiwanie na sygna³y SIGHUP i SIGQUIT...\n");



    // G³ówna pêtla programu
    while (1) {
        pause(); // Oczekiwanie na sygna³y
    }

    // Od³¹czenie pamiêci wspó³dzielonej (teoretycznie, bo pause() czeka na sygna³y)
    detach_shared_memory(shm);

    return 0;
}
