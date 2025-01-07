#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
    pid_t init_pid, pszczelarz_pid, krolowa_pid;

    // Uruchamianie programu init
    printf("Uruchamianie programu init...\n");
    init_pid = fork();
    if (init_pid == 0) {
        execl("./init", "./init", NULL);
        perror("Nie uda³o siê uruchomiæ init");
        exit(EXIT_FAILURE);
    }
    else if (init_pid < 0) {
        perror("B³¹d podczas forka dla init");
        exit(EXIT_FAILURE);
    }


    sleep(1);

    printf("Uruchamianie programu pszczelarz...\n");
    pszczelarz_pid = fork();
    if (pszczelarz_pid == 0) {
        execl("./pszczelarz", "./pszczelarz", NULL);
        perror("Nie uda³o siê uruchomiæ pszczelarz");
        exit(EXIT_FAILURE);
    }
    else if (pszczelarz_pid < 0) {
        perror("B³¹d podczas forka dla pszczelarz");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    printf("Uruchamianie programu krolowa...\n");
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

    printf("Oczekiwanie na zakoñczenie pszczelarz i krolowa...\n");

    waitpid(pszczelarz_pid, NULL, 0);
    printf("Program pszczelarz zakoñczony.\n");

    waitpid(krolowa_pid, NULL, 0); 
    printf("Program krolowa zakoñczony.\n");

    printf("Koñczenie programu init, jeœli dzia³a...\n");
    kill(init_pid, SIGTERM);
    waitpid(init_pid, NULL, 0);
    printf("Program init zakoñczony.\n");

    printf("Wszystkie procesy zakoñczone.\n");
    return 0;
}
