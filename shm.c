/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include "shm.h"
*/
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "shm.h"
// Definicja struktury dla pami�ci wsp�dzielonej
/*
struct SharedMemory {
    int P; // Maksymalna liczba pszcz� w ulu
    int N; // Maksymalna liczba pszcz� w populacji
};
*/
// Funkcja tworzy segment pami�ci wsp�dzielonej
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
    int shmid = shmget(key, size, IPC_CREAT | 0666);
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