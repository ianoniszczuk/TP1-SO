#include "sharedMemory.h"
#include "errorHandling.h"
#include <stdio.h>
#include <stdlib.h>

void *mapSharedMemory(const char *name, size_t size, int *fd, int mode, int flags) {
    int create_flag = (flags | O_CREAT) == flags;
    int write_flag = (flags | O_RDWR) == flags;
    
    *fd = shm_open(name, flags, mode);
    if(*fd == -1) {
        ERROR_EXIT("shm_open");
    }

    if(create_flag) {
        if(ftruncate(*fd, size) == -1) {
            destroySharedMemory(name, NULL, *fd, size);
            ERROR_EXIT("ftruncate");
        }
    }

    void *shmBasePtr = mmap(NULL, size, (write_flag ? PROT_WRITE : 0)  | PROT_READ, MAP_SHARED, *fd, 0);
    if(shmBasePtr == MAP_FAILED) {
        if(create_flag) {
            destroySharedMemory(name, shmBasePtr, *fd, size);
        } else {
            closeSharedMemory(NULL, *fd, size);
        }
        ERROR_EXIT("mmap");
    }
    return shmBasePtr;
}

void *createSharedMemory(const char *name, size_t size, int *fd, int mode) {
    return mapSharedMemory(name, size, fd, mode, O_CREAT | O_RDWR);
}

void *openSharedMemory(const char *name, size_t size, int *fd, int flags) {
    return mapSharedMemory(name, size, fd, 0, flags);
}

void closeSharedMemory(void *shmPtr, int fd, size_t size) {
    if(shmPtr != NULL && shmPtr != MAP_FAILED) {
        munmap(shmPtr, size);
    }
    if(fd >= 0) {
        close(fd);
    }
}

void destroySharedMemory(const char *name, void *shmPtr, int fd, size_t size) {
    closeSharedMemory(shmPtr, fd, size);
    if(name != NULL) {
        shm_unlink(name);
    }
}