#ifndef SHM_H
#define SHM_H

#include <sys/types.h>

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

#endif // SHM_H
