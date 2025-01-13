#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include "shm.h"
//#include "sem.h"

pid_t queen_pid; // PID królowej
pid_t init_pid;  // PID procesu init
int semid;       // ID semaforów
int shmid;       // ID pamiêci wspó³dzielonej
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
    printf("[MASTER] ###### Czyszczenie zasobów... ###### \n");
    /*
    // Zakoñczenie procesu królowej
    if (queen_pid > 0) {
        kill(queen_pid, SIGTERM);
        printf("[MASTER] Wys³ano SIGTERM do królowej (PID: %d).\n", queen_pid);
    }

    // Zakoñczenie procesu init
    if (init_pid > 0) {
        kill(init_pid, SIGTERM);
        printf("[MASTER] Wys³ano SIGTERM do init (PID: %d).\n", init_pid);
    }
    */
    // Usuniêcie pamiêci wspó³dzielonej
    if (shmid > 0) {
        shmctl(shmid, IPC_RMID, NULL);
        printf("[MASTER] Zwolniono pamiêæ wspó³dzielon¹. \n");
    }

    // Usuniêcie semaforów
    if (semid > 0) {
        semctl(semid, 0, IPC_RMID);
        //free_semaphore(semid, 0);
        //free_semaphore(semid, 0);
        printf("[MASTER] Zwolniono semafory.\n");
    }
    printf("[MASTER] Program pszczelarz zakoñczony.\n");
    printf("[MASTER] Program krolowa zakoñczony.\n");
    printf("[MASTER] Program init zakoñczony.\n");
    printf("[MASTER] Program zakoñczony.\n");
    exit(0);
}

void handle_sigint(int sig) {
    printf("[MASTER] Otrzymano SIGINT (Ctrl + C). Koñczê dzia³anie... \n");
    cleanup_and_exit();
}


int main() {
    signal(SIGINT, handle_sigint);
    pid_t init_pid, pszczelarz_pid, krolowa_pid;
    // Inicjalizacja zasobów
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

    printf("[MASTER] Uruchamianie init...\n");
    init_pid = fork();
    if (init_pid == 0) {
        execl("./init", "init", NULL);
        perror("Nie uda³o siê uruchomiæ init");
        exit(EXIT_FAILURE);
    }

    // #####
    /*
    pid_t init_pid, pszczelarz_pid, krolowa_pid;
    // Uruchamianie programu init
    printf("Uruchamianie programu init...\n");
    init_pid = fork();
    if (init_pid == 0) {
        execl("./init", "./init", NULL);
        perror("Nie uda³o siê uruchomiæ init");
        exit(EXIT_FAILURE);
    } else if (init_pid < 0) {
        perror("B³¹d podczas forka dla init");
        exit(EXIT_FAILURE);
    }
    */
    sleep(1);
    /*
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(shm_key, sizeof(struct SharedMemory), 0600);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    struct SharedMemory* shm = (struct SharedMemory*) shmat(shmid, NULL, 0);
    if (shm == (void*) -1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Zapytaj u¿ytkownika o nowy limit populacji
    int population_limit;
    printf("Podaj nowy limit populacji (N): ");
    if (scanf("%d", &population_limit) != 1 || population_limit <= 0) {
        fprintf(stderr, "Niepoprawny limit populacji. Podaj liczbê ca³kowit¹ wiêksz¹ od 0.\n");
        shmdt(shm);
        exit(EXIT_FAILURE);
    }

    // Aktualizacja limitu populacji i ula w pamiêci wspó³dzielonej
    shm->N = population_limit;
    shm->P = population_limit / 2; // Zaktualizuj limit pszczó³ w ulu
    printf("Zaktualizowano limity: N = %d, P = %d\n", shm->N, shm->P);
    */
    // Uruchamianie programu pszczelarz
    printf("[MASTER] Uruchamianie programu pszczelarz...\n");
    pszczelarz_pid = fork();
    if (pszczelarz_pid == 0) {
        // Proces potomny dla pszczelarz
        execl("./pszczelarz", "./pszczelarz", NULL);
        perror("Nie uda³o siê uruchomiæ pszczelarz");
        exit(EXIT_FAILURE);
    }
    else if (pszczelarz_pid < 0) {
        perror("B³¹d podczas forka dla pszczelarz");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    printf("[MASTER] Uruchamianie programu krolowa...\n");
    krolowa_pid = fork();
    if (krolowa_pid == 0) {
        execl("./krolowa", "./krolowa", NULL);
        perror("Nie uda³o siê uruchomiæ krolowa");
        exit(EXIT_FAILURE);
    }
    else if (krolowa_pid < 0) {
        perror("B³¹d podczas forka dla krolowa");
        exit(EXIT_FAILURE);
    }
    // Opcjolanie czekanie na zakonczenie
    printf("Oczekiwanie na zakoñczenie pszczelarz i krolowa...\n");

    //waitpid(pszczelarz_pid, NULL, 0);
    //printf("Program pszczelarz zakoñczony.\n");

    waitpid(krolowa_pid, NULL, 0);
    printf("Program krolowa zakoñczony.\n");

    printf(" [MASTER] Koñczenie programu init, jeœli dzia³a...\n");
    kill(init_pid, SIGTERM);
    waitpid(init_pid, NULL, 0);
    printf("[MASTER] Program init zakoñczony.\n");

    printf("[MASTER] Wszystkie procesy zakoñczone.\n");
    return 0;
}
