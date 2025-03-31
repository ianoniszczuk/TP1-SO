#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

void *createSharedMemory(char *name, size_t size, int *fd);
void *openSharedMemory(char *name, size_t size, int *fd);
void closeSharedMemory(void *shm_ptr, size_t size, int fd);
void destroySharedMemory(char *name, void *shm_ptr, size_t size, int fd);

#endif