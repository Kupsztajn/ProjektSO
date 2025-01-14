#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include "shm.h"

// Funkcja tworzy segment pami�ci wsp�dzielonej
void zbior_sem_mem(int* shmid, struct SharedMemory** shm, int* semid) {
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja pami�ci wsp�dzielonej
    *shmid = shmget(shm_key, sizeof(struct SharedMemory), IPC_CREAT | 0600);
    if (*shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    *shm = (struct SharedMemory*)attach_shared_memory(*shmid);
    if (*shm == NULL) {
        perror("attach_shared_memory failed");
        exit(EXIT_FAILURE);
    }

    // Klucz do semafor�w
    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semafor�w
    *semid = semget(sem_key, 6, IPC_CREAT | 0600);
    if (*semid == -1) {
        perror("semget failed");
        detach_shared_memory(*shm);
        exit(EXIT_FAILURE);
    }
}



int create_shared_memory(const char* pathname, int proj_id, size_t size) {
    // Generowanie klucza za pomoc� ftok
    printf("Tworzenie klucza IPC z %s i %d\n", pathname, proj_id);
    key_t key = ftok(pathname, proj_id);
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    printf("Generowany klucz: %d\n", key);
    // Tworzenie segmentu pami�ci wsp�dzielonej
    int shmid = shmget(key, size, IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget failed");
        printf("Rozmiar segmentu: %lu\n", size);
        printf("Klucz IPC: %d\n", key);
        exit(EXIT_FAILURE);
    }

    return shmid;
}

// Mapuje pami�� wsp�dzielon� do przestrzeni adresowej procesu
void* attach_shared_memory(int shmid) {
    void* addr = shmat(shmid, NULL, 0);
    if (addr == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    return addr;
}

// Od��cza segment pami�ci wsp�dzielonej od procesu
void detach_shared_memory(void* addr) {
    if (shmdt(addr) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}

// Usuwa segment pami�ci wsp�dzielonej
void destroy_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(EXIT_FAILURE);
    }
}

int free_semaphore(int semID, int liczba) {
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

// Funkcja inicjalizuje warto�� semafora
void init_semaphore(int semID, int liczba, int wartosc) {
    if (semctl(semID, liczba, SETVAL, wartosc) == -1) {
        perror("Blad inicjalizacji tablicy smeaforow (inicjalizujSemafor):");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Poprawnie zainicjalizowano semafor %d na wartosc %d \n", liczba, wartosc);
    }
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

// Funkcja wykonuje operacj� WAIT na semaforze (P) #### delete
/*
int waitSemafor(int semID, int liczba, int flagi) {
    struct sembuf operacje[1];
    operacje[0].sem_num = liczba;
    operacje[0].sem_op = -1;       // Zmniejsz warto�� semafora
    operacje[0].sem_flg = flagi;   // Flagi, np. SEM_UNDO

    if (semop(semID, operacje, 1) == -1) {
        perror("Blad semop (waitSemafor):");
        return -1; // Zwr�� b��d, aby mo�na by�o to obs�u�y� wy�ej
    }
    else
    {
    printf("Opuszczono semafor %d \n", liczba);
    }

    return 0; // Operacja udana
}

// Funkcja wykonuje operacj� SIGNAL na semaforze (V)
void signalSemafor(int semID, int liczba) {
    struct sembuf operacje[1];
    operacje[0].sem_num = liczba;
    operacje[0].sem_op = 1;        // Zwi�ksz warto�� semafora
    operacje[0].sem_flg = 0;       // Opcjonalne flagi

    if (semop(semID, operacje, 1) == -1) {
        perror("Blad semop (signalSemafor):");
    }
    else
    {
        printf("Podniesiono semafor %d \n", liczba);
    }

}

// Funkcja zwraca aktualn� warto�� semafora
int valueSemafor(int semID, int liczba) {
    int wartosc = semctl(semID, liczba, GETVAL, NULL);
    if (wartosc == -1) {
        perror("Blad semctl (valueSemafor):");
    }
    return wartosc;
}

// #######
/*
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
*/