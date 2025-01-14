#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include "shm.h"

// Funkcja tworzy segment pamiêci wspó³dzielonej
void zbior_sem_mem(int* shmid, struct SharedMemory** shm, int* semid) {
    key_t shm_key = ftok("/tmp", 'A');
    if (shm_key == -1) {
        perror("ftok failed for shared memory");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja pamiêci wspó³dzielonej
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

    // Klucz do semaforów
    key_t sem_key = ftok("/tmp", 'B');
    if (sem_key == -1) {
        perror("ftok failed for semaphores");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semaforów
    *semid = semget(sem_key, 6, IPC_CREAT | 0600);
    if (*semid == -1) {
        perror("semget failed");
        detach_shared_memory(*shm);
        exit(EXIT_FAILURE);
    }
}



int create_shared_memory(const char* pathname, int proj_id, size_t size) {
    // Generowanie klucza za pomoc¹ ftok
    printf("Tworzenie klucza IPC z %s i %d\n", pathname, proj_id);
    key_t key = ftok(pathname, proj_id);
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    printf("Generowany klucz: %d\n", key);
    // Tworzenie segmentu pamiêci wspó³dzielonej
    int shmid = shmget(key, size, IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget failed");
        printf("Rozmiar segmentu: %lu\n", size);
        printf("Klucz IPC: %d\n", key);
        exit(EXIT_FAILURE);
    }

    return shmid;
}

// Mapuje pamiêæ wspó³dzielon¹ do przestrzeni adresowej procesu
void* attach_shared_memory(int shmid) {
    void* addr = shmat(shmid, NULL, 0);
    if (addr == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    return addr;
}

// Od³¹cza segment pamiêci wspó³dzielonej od procesu
void detach_shared_memory(void* addr) {
    if (shmdt(addr) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}

// Usuwa segment pamiêci wspó³dzielonej
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

    return wynik;
}

// Funkcja inicjalizuje wartoœæ semafora
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