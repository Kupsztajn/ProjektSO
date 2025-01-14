#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include "shm.h"

pid_t queen_pid; // PID krolowej
pid_t init_pid;  // PID procesu init
int semid;       // ID semaforow
int shmid;       // ID pamieci wspoldzielonej

void cleanup_and_exit() {
    printf("[MASTER] ###### Czyszczenie zasob�w... ###### \n");

    // Usuniecie pamieci wspoldzielonej
    if (shmid > 0) {
        destroy_shared_memory(shmid);
        printf("[MASTER] Zwolniono pami�� wsp�dzielon�. \n");
    }

    // Usuniecie semaforow
    if (semid > 0) {
        free_semaphore(semid, 0);
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

int main() {
    // rejestracja sygnalu ctrl + c
    signal(SIGINT, handle_sigint);

    pid_t init_pid, pszczelarz_pid, krolowa_pid;
    struct SharedMemory* shm;
    int limit;

    // Wywolanie funkcji do inicjalizacji zasobow IPC
    zbior_sem_mem(&shmid, &shm, &semid);

    // Wczytywanie limitu populacji od uzytkownika
    while (1) {
        printf("Podaj limit populacji (liczba ca�kowita wi�ksza od 1): ");
        if (scanf("%d", &limit) == 1 && limit > 1) {
            // Wartosc poprawna
            break;
        }
        else {
            // Wartosc niepoprawna
            printf("Niepoprawna warto��! Spr�buj ponownie.\n");

            // Wyczysc bufor wejsciowy
            while (getchar() != '\n');
        }
    }

    printf("[MASTER] Uruchamianie init...\n");

    // Proces init
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

    // Proces potomny dla pszczelarz
    pszczelarz_pid = fork();

    if (pszczelarz_pid == 0) {
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

    // Proces krolowa
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

    // Opcjonalne czekanie na zakonczenie / czekanie na sygnal ctrl + c
    printf("Oczekiwanie na zako�czenie pszczelarz i krolowa...\n");

    waitpid(krolowa_pid, NULL, 0);
    printf("Program krolowa zako�czony.\n");

    printf(" [MASTER] Ko�czenie programu init, je�li dzia�a...\n");

    kill(init_pid, SIGTERM);
    waitpid(init_pid, NULL, 0);

    printf("[MASTER] Program init zako�czony.\n");

    printf("[MASTER] Wszystkie procesy zako�czone.\n");
    return 0;
}
