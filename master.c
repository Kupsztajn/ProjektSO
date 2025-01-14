#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include "shm.h"

pid_t queen_pid; // PID kr�lowej
pid_t init_pid;  // PID procesu init
int semid;       // ID semafor�w
int shmid;       // ID pami�ci wsp�dzielonej
/*
int free_semaphore(int semID, int liczba) {
    int wynik = semctl(semid, 0, IPC_RMID, NULL);
    if (wynik == -1) {
        perror("Blad zwalniania tablicy semaforow (zwolnijSemafor):");
    }
    else
    {
    printf("Zwolniono semafory \n");
    }
    return wynik;
}
*/
void cleanup_and_exit() {
    printf("[MASTER] ###### Czyszczenie zasob�w... ###### \n");
    /*
    // Zako�czenie procesu kr�lowej
    if (queen_pid > 0) {
        kill(queen_pid, SIGTERM);
        printf("[MASTER] Wys�ano SIGTERM do kr�lowej (PID: %d).\n", queen_pid);
    }

    // Zako�czenie procesu init
    if (init_pid > 0) {
        kill(init_pid, SIGTERM);
        printf("[MASTER] Wys�ano SIGTERM do init (PID: %d).\n", init_pid);
    }
    */
    // Usuni�cie pami�ci wsp�dzielonej
    if (shmid > 0) {
        shmctl(shmid, IPC_RMID, NULL);
        printf("[MASTER] Zwolniono pami�� wsp�dzielon�. \n");
    }

    // Usuni�cie semafor�w
    if (semid > 0) {
        semctl(semid, 0, IPC_RMID);
        //free_semaphore(semid, 0);
        //free_semaphore(semid, 0);
        printf("[MASTER] Zwolniono semafory.\n");
    }
    sleep(1);
    printf("[MASTER] Program pszczelarz zako�czony.\n");
    printf("[MASTER] Program krolowa zako�czony.\n");
    printf("[MASTER] Program init zako�czony.\n");
    sleep(1);
    printf("[MASTER] Program zako�czony.\n");
    exit(0);
}

void handle_sigint(int sig) {
    printf("[MASTER] Otrzymano SIGINT (Ctrl + C). Ko�cz� dzia�anie... \n");
    cleanup_and_exit();
}

//int shmid, semid;
//struct SharedMemory* shm;

// Wywo�anie funkcji do inicjalizacji zasob�w IPC
//zbior_sem_mem(&shmid, &shm, &semid);


int main() {
    signal(SIGINT, handle_sigint);
    pid_t init_pid, pszczelarz_pid, krolowa_pid;

    //int shmid, semid;
    //struct SharedMemory* shm;

    // Wywo�anie funkcji do inicjalizacji zasob�w IPC
    //zbior_sem_mem2(&shmid, &shm, &semid);

    // Inicjalizacja zasob�w

    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    shmid = shmget(shm_key, sizeof(struct SharedMemory), IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    struct SharedMemory* shm = (struct SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (void*)-1) {
        perror("Nie uda�o si� pod��czy� segmentu pami�ci wsp�dzielonej");
        exit(EXIT_FAILURE);
    }

    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    semid = semget(sem_key, 6, IPC_CREAT | 0600);
    if (semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    int limit;

    // Wczytywanie limitu populacji od u�ytkownika
    while (1) {
        printf("Podaj limit populacji (liczba ca�kowita wi�ksza od 1): ");
        if (scanf("%d", &limit) == 1 && limit > 1) {
            // Warto�� poprawna
            break;
        }
        else {
            // Warto�� niepoprawna
            printf("Niepoprawna warto��! Spr�buj ponownie.\n");

            // Wyczy�� bufor wej�ciowy
            while (getchar() != '\n');
        }
    }

    printf("[MASTER] Uruchamianie init...\n");
    init_pid = fork();
    if (init_pid == 0) {
        execl("./init", "init", NULL);
        perror("Nie uda�o si� uruchomi� init");
        exit(EXIT_FAILURE);
    }

    shm->N = limit;
    shm->P = limit / 2;
    sleep(1);
    printf("[MASTER] Przypisano limit populacji: %d\n", limit);
    printf("[MASTER] Przypisano limit ula: %d\n", shm->P);

    sleep(2);

    // Uruchamianie programu pszczelarz
    printf("[MASTER] Uruchamianie programu pszczelarz...\n");
    pszczelarz_pid = fork();
    if (pszczelarz_pid == 0) {
        // Proces potomny dla pszczelarz
        execl("./pszczelarz", "./pszczelarz", NULL);
        perror("Nie uda�o si� uruchomi� pszczelarz");
        exit(EXIT_FAILURE);
    }
    else if (pszczelarz_pid < 0) {
        perror("B��d podczas forka dla pszczelarz");
        exit(EXIT_FAILURE);
    }

    sleep(2);

    printf("[MASTER] Uruchamianie programu krolowa...\n");
    krolowa_pid = fork();
    if (krolowa_pid == 0) {
        execl("./krolowa", "./krolowa", NULL);
        perror("Nie uda�o si� uruchomi� krolowa");
        exit(EXIT_FAILURE);
    }
    else if (krolowa_pid < 0) {
        perror("B��d podczas forka dla krolowa");
        exit(EXIT_FAILURE);
    }
    // Opcjolanie czekanie na zakonczenie
    printf("Oczekiwanie na zako�czenie pszczelarz i krolowa...\n");

    //waitpid(pszczelarz_pid, NULL, 0);
    //printf("Program pszczelarz zako�czony.\n");

    waitpid(krolowa_pid, NULL, 0);
    printf("Program krolowa zako�czony.\n");

    printf(" [MASTER] Ko�czenie programu init, je�li dzia�a...\n");
    kill(init_pid, SIGTERM);
    waitpid(init_pid, NULL, 0);
    printf("[MASTER] Program init zako�czony.\n");

    printf("[MASTER] Wszystkie procesy zako�czone.\n");
    return 0;
}
