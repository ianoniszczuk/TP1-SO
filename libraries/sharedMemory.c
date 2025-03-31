#include "sharedMemory.h"
#include "errorHandling.h"
#include <stdio.h>
#include <stdlib.h>

void *mapSharedMemory(const char *name, size_t size, int *fd, int create_flag) {
    int flags = O_RDWR | (create_flag ? O_CREAT : 0);
    
    *fd = shm_open(name, flags, 0666);
    if(*fd == -1) {
        ERROR_EXIT("shm_open");
    }

    if(create_flag) {
        if(ftruncate(*fd, size) == -1) {
            destroySharedMemory(name, NULL, *fd, size);
            ERROR_EXIT("ftruncate");
        }
    }

    void *shmBasePtr = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, *fd, 0);
    if(shmBasePtr == MAP_FAILED) {
        if(create_flag) {
            destroySharedMemory(name, shmBasePtr, *fd,size);
            shm_unlink(name);
        } else {
            closeSharedMemory(NULL,*fd, size);
        }
        ERROR_EXIT("mmap");
    }
    return shmBasePtr;
}

void *createSharedMemory(char *name, size_t size, int *fd) {
    return mapSharedMemory(name, size, fd, 1);
}

void *openSharedMemory(char *name, size_t size, int *fd) {
    return mapSharedMemory(name, size, fd, 0);
}

void closeSharedMemory(void *shmPtr, size_t size, int fd) {
    if(shmPtr != NULL && shmPtr != MAP_FAILED) {
        munmap(shmPtr, size);
    }
    if(fd >= 0) {
        close(fd);
    }
}

void destroySharedMemory(char *name, void *shmPtr, size_t size, int fd) {
    closeSharedMemory(shmPtr, size, fd);
    shm_unlink(fd);
}