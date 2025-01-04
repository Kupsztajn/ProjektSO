#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

// Funkcja alokuje semafor i zwraca jego ID
int alokujSemafor(key_t klucz, int liczba, int flagi) {
    int semID = semget(klucz, liczba, flagi);
    if (semID == -1) {
        perror("Blad semget (alokujSemafor):");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Tablica semaforow zostal zaalokowana w liczbie %d \n", liczba);
    }
    return semID;
}

int zwolnijSemafor(int semID, int liczba) {
    int wynik = semctl(semID, liczba, IPC_RMID, NULL);
    if (wynik == -1) {
        perror("Blad zwalniania tablicy semaforow (zwolnijSemafor):");
    }
    else
    {
        printf("Zwolniono semafory \n");
    }
    return wynik;
}

// Funkcja inicjalizuje wartoœæ semafora
void inicjalizujSemafor(int semID, int liczba, int wartosc) {
    if (semctl(semID, liczba, SETVAL, wartosc) == -1) {
        perror("Blad inicjalizacji tablicy smeaforow (inicjalizujSemafor):");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Poprawnie zainicjalizowano semafor %d na wartosc %d \n", liczba, wartosc);
    }
}

// Funkcja wykonuje operacjê WAIT na semaforze (P)
int waitSemafor(int semID, int liczba, int flagi) {
    struct sembuf operacje[1];
    operacje[0].sem_num = liczba;
    operacje[0].sem_op = -1;       // Zmniejsz wartoœæ semafora
    operacje[0].sem_flg = flagi;   // Flagi, np. SEM_UNDO

    if (semop(semID, operacje, 1) == -1) {
        perror("Blad semop (waitSemafor):");
        return -1; // Zwróæ b³¹d, aby mo¿na by³o to obs³u¿yæ wy¿ej
    }
    else
    {
        printf("Opuszczono semafor %d \n", liczba);
    }

    return 0; // Operacja udana
}

// Funkcja wykonuje operacjê SIGNAL na semaforze (V)
void signalSemafor(int semID, int liczba) {
    struct sembuf operacje[1];
    operacje[0].sem_num = liczba;
    operacje[0].sem_op = 1;        // Zwiêksz wartoœæ semafora
    operacje[0].sem_flg = 0;       // Opcjonalne flagi

    if (semop(semID, operacje, 1) == -1) {
        perror("Blad semop (signalSemafor):");
    }
    else
    {
        printf("Podniesiono semafor %d \n", liczba);
    }

}

// Funkcja zwraca aktualn¹ wartoœæ semafora
int valueSemafor(int semID, int liczba) {
    int wartosc = semctl(semID, liczba, GETVAL, NULL);
    if (wartosc == -1) {
        perror("Blad semctl (valueSemafor):");
    }
    return wartosc;
}

