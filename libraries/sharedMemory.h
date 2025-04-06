#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// Function declarations
void *createSharedMemory(const char *name, size_t size, int *fd);
void *openSharedMemory(const char *name, size_t size, int *fd);
void closeSharedMemory(void *shmPtr, int fd, size_t size);
void destroySharedMemory(const char *name, void *shmPtr, int fd, size_t size);

#endif