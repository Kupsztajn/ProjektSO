#ifndef SHM_H
#define SHM_H

#include <sys/types.h>

struct SharedMemory {
    int P; // Maksymalna liczba pszczó³ w ulu
    int N; // Maksymalna liczba pszczó³ w populacji
    int il;
};

int create_shared_memory(const char* pathname, int proj_id, size_t size);
void* attach_shared_memory(int shmid);
void detach_shared_memory(void* addr);
void destroy_shared_memory(int shmid);

#endif
