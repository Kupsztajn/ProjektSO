#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "shm.h"

/*
struct SharedMemory {
    int P; // Maksymalna liczba pszcz� w ulu
    int N; // Maksymalna liczba pszcz� w populacji
};
*/
// Funkcja tworzy segment pami�ci wsp�dzielonej
void zbior_sem_mem(int* shmid, struct SharedMemory** shm, int* semid) {
    // Klucz do pami�ci wsp�dzielonej
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