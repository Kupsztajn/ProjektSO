
#include <sys/types.h>



int create_shared_memory(const char* pathname, int proj_id, size_t size);


void* attach_shared_memory(int shmid);


void detach_shared_memory(void* addr);

void destroy_shared_memory(int shmid);

