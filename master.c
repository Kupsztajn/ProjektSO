#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "shm.h"
int main() {
    pid_t init_pid, pszczelarz_pid, krolowa_pid;

    // Uruchamianie programu init
    printf("Uruchamianie programu init...\n");
    init_pid = fork();
    if (init_pid == 0) {
        // Proces potomny dla init
        execl("./init", "./init", NULL);
        perror("Nie uda�o si� uruchomi� init");
        exit(EXIT_FAILURE);
    }
    else if (init_pid < 0) {
        perror("B��d podczas forka dla init");
        exit(EXIT_FAILURE);
    }

    // Poczekaj chwil�, aby init zainicjowa� pami�� wsp�dzielon� i semafory
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

    // Zapytaj u�ytkownika o nowy limit populacji
    int population_limit;
    printf("Podaj nowy limit populacji (N): ");
    if (scanf("%d", &population_limit) != 1 || population_limit <= 0) {
        fprintf(stderr, "Niepoprawny limit populacji. Podaj liczb� ca�kowit� wi�ksz� od 0.\n");
        shmdt(shm);
        exit(EXIT_FAILURE);
    }

    // Aktualizacja limitu populacji i ula w pami�ci wsp�dzielonej
    shm->N = population_limit;
    shm->P = population_limit / 2; // Zaktualizuj limit pszcz� w ulu
    printf("Zaktualizowano limity: N = %d, P = %d\n", shm->N, shm->P);
    */
    // Uruchamianie programu pszczelarz
    printf("Uruchamianie programu pszczelarz...\n");
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
    sleep(1);
    // Uruchamianie programu krolowa
    printf("Uruchamianie programu krolowa...\n");
    krolowa_pid = fork();
    if (krolowa_pid == 0) {
        // Proces potomny dla krolowa
        execl("./krolowa", "./krolowa", NULL);
        perror("Nie uda�o si� uruchomi� krolowa");
        exit(EXIT_FAILURE);
    }
    else if (krolowa_pid < 0) {
        perror("B��d podczas forka dla krolowa");
        exit(EXIT_FAILURE);
    }

    // Czekanie na zako�czenie pszczelarz i krolowa
    printf("Oczekiwanie na zako�czenie pszczelarz i krolowa...\n");

    waitpid(pszczelarz_pid, NULL, 0); // Czekaj na zako�czenie pszczelarz
    printf("Program pszczelarz zako�czony.\n");

    waitpid(krolowa_pid, NULL, 0); // Czekaj na zako�czenie krolowa
    printf("Program krolowa zako�czony.\n");

    // Opcjonalne: zako�czenie programu init, je�li nadal dzia�a
    printf("Ko�czenie programu init, je�li dzia�a...\n");
    kill(init_pid, SIGTERM); // Wysy�a sygna� zako�czenia do init
    waitpid(init_pid, NULL, 0);
    printf("Program init zako�czony.\n");

    printf("Wszystkie procesy zako�czone.\n");
    return 0;
}
