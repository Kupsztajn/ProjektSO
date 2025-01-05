#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"
#include "sem.h"

#define VISITS 5

// Indeksy semaforów w tablicy
#define SEM_ENT1 0
//#define SEM_POM 5
#define SEM_ENT2 1 
#define SEM_ULE  2 
#define SEM_POP  3 
#define SEM_KROL 4
#define SEM_LOCK 5

void bee_logic(int semid, int* P, int* shm, int* nadmiarULE, int* nadmiarPOP);

int main()
{
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(shm_key, sizeof(struct SharedMemory), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    struct SharedMemory* shm = (struct SharedMemory*)attach_shared_memory(shmid);
    if (!shm) {
        perror("attach_shared_memory failed");
        exit(EXIT_FAILURE);
    }
    shm = (struct SharedMemory*)attach_shared_memory(shmid);

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

    bee_logic(semid, &shm->P, &shm->N, &shm->nadmiar_ULE, &shm->nadmiar_POP);

    
	return 0;
}

int release_semaphore(int semid, int sem_num) {
    struct sembuf op = { sem_num, 1, 0 };
    if (semop(semid, &op, 1) == -1) {
        perror("Failed to release semaphore");
        return -1;
    }
    return 0;
}

int acquire_semaphore(int semid, int sem_num) {
    struct sembuf op = { sem_num, -1, 0 };
    if (semop(semid, &op, 1) == -1) {
        perror("Failed to acquire semaphore");
        return -1;
    }
    return 0;
}

void release_entrance(int semid, int sem_entrance) {
    struct sembuf op_increase_entrance = { sem_entrance, 1, 0 };
    if (semop(semid, &op_increase_entrance, 1) == -1) {
        perror("Failed to release entrance semafor");
    }
}


void bee_logic(int semid, int* P, int* N, int* nadmiarULE, int* nadmiarPOP) {
    int odwiedziny = VISITS;
    while (odwiedziny--) {

        if (*nadmiarPOP > 0 || *nadmiarULE > 0) {
            printf("\t [Pszczola] NADMIAR PSZCZOL, PSZCZOLA UMIERA W ULU, BRAK DLA NIEJ MIEJSCA");
            if (*nadmiarULE > 0) { (*nadmiarULE)--; (*nadmiarPOP)--; }

            return 0;
        }

        sleep(rand() % 5 + 1); 

        
        int entrance = rand() % 2;
        int sem_entrance = (entrance == 0) ? SEM_ENT1 : SEM_ENT2;

      


        if (semop(semid, exit_ops, 2) == -1) {
            perror("semop failed during exit");
            continue;
        }

        printf("\t [Pszczola] Pszczo³a opuszcza ul przez wejœcie %d.\n", entrance + 1);
        release_entrance(semid, entrance);
        // Symulacja pracy na zewn¹trz
        sleep(rand() % 5 + 1);


        if (*nadmiarPOP > 0) {
            printf("\t [Pszczola] NADMIAR PSZCZOL, PSZCZOLA UMIERA, BRAK DLA NIEJ MIEJSCA");
            (*nadmiarPOP)--;

            return 0;
        }


        // Wybór wejœcia przy powrocie
        entrance = rand() % 2;
        sem_entrance = (entrance == 0) ? SEM_ENT1 : SEM_ENT2;

        // Zajêcie wejœcia
        struct sembuf enter_ops[] = {
            {sem_entrance, -1, 0}, // Zajêcie wejœcia
            {SEM_ULE, -1, 0}        // zajecie miejsca w ulu
        };
        if (semop(semid, enter_ops, 2) == -1) {
            perror("semop failed during exit");
            continue;
        }


        printf("\t [Pszczola] Pszczo³a wesz³a do ula przez wejœcie %d.\n", entrance + 1);

        // Zwolnij wejœcie
        release_semaphore(semid, sem_entrance);

        if (*nadmiarPOP > 0 || *nadmiarULE > 0) {
            printf("\t [Pszczola] NADMIAR PSZCZOL, PSZCZOLA UMIERA W ULU, BRAK DLA NIEJ MIEJSCA");
            if (*nadmiarULE > 0) { (*nadmiarULE)--; (*nadmiarPOP)--; }

            return 0;
        }

    }

    if (semop(semid, death, 2) == -1) {
        perror("semop failed during exit");
    }

    /*
    if (semop(semid, death, 2) != -1 && *nadmiarULE == 0 && *nadmiarPOP == 0) {
        printf("\t [Pszczola] Brak nadmiaru wszystko zgodnie z planem \n ");
    }
    else if (semop(semid, death_noPOP, 2) != -1 && *nadmiarULE == 0 && *nadmiarPOP > 0){
        printf("\t [Pszczola] Nadmiar pszczol w Populacji nie zwiekszam semafora POP \n ");
        (*nadmiarPOP)--;
    }
    else if (semop(semid, death_noULE, 2) != -1 && *nadmiarULE > 0 && *nadmiarPOP == 0){
        printf("\t [Pszczola] Nadmiar pszczol w ulu nie zwiekszam semafora ULE \n ");
        (*nadmiarULE)--;
    }
    else if (semop(semid, death_noULE_noPOP, 2) != -1 && *nadmiarULE > 0 && *nadmiarPOP > 0){
        printf("\t [Pszczola] Nadmiar pszczol w ulu i populacji nie zwiekszam semafora ULE i POP \n ");
        (*nadmiarULE)--;
        (*nadmiarPOP)--;
    }
    */
    printf("\t [Pszczola] Pszczola umarla Semafor_ULE: %d \n", semctl(semid, SEM_ULE, GETVAL));
    return 0;

}