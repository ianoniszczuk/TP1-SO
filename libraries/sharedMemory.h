#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// Function declarations
void *mapSharedMemory(const char *name, size_t size, int *fd, int mode, int flags);
void *createSharedMemory(const char *name, size_t size, int *fd, int mode);
void *openSharedMemory(const char *name, size_t size, int *fd, int flags);
void closeSharedMemory(void *shmPtr, int fd, size_t size);
void destroySharedMemory(const char *name, void *shmPtr, int fd, size_t size);

#endif