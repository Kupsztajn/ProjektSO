#ifndef SEM_H
#define SEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

//int alokujSemafor(key_t klucz, int number, int flagi);
void init_semaphore(int semID, int number, int val);
//int zwolnijSemafor(int semID, int number);
//int waitSemafor(int semID, int number, int flags);
//void signalSemafor(int semID, int number);
//int valueSemafor(int semID, int number);
int release_semaphore(int semid, int sem_num);

int acquire_semaphore(int semid, int sem_num);

void release_entrance(int semid, int sem_entrance);
#endif // SEM_H