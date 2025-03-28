#ifndef __SHAREDMEMORY_H
#define __SHAREDMEMORY_H

#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

void *createSharedMemory(char *name, int size, int *fd);
void *openSharedMemory(char *name, int size, int *fd);
void closeSharedMemory(void *shm_ptr, int size, int fd);
void destroySharedMemory(const char *name, void *shm_ptr, int fd, int size);

#endif