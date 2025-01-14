#ifndef SHM_H
#define SHM_H

#include <sys/types.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

struct SharedMemory {
    int P;
    int N;
    int nadmiar_ULE;
    int nadmiar_POP;
};

int create_shared_memory(const char* pathname, int proj_id, size_t size);

void zbior_sem_mem(int* shmid, struct SharedMemory** shm, int* semid);

void* attach_shared_memory(int shmid);

void detach_shared_memory(void* addr);

void destroy_shared_memory(int shmid);

void init_semaphore(int semID, int number, int val);

int free_semaphore(int semID, int number);

int release_semaphore(int semid, int sem_num);

int acquire_semaphore(int semid, int sem_num);

void release_entrance(int semid, int sem_entrance);

#endif
