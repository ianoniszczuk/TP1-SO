#include "sharedMemoryAdt.h"
#include "errorHandling.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void *mapSharedMemory(const char *name, size_t size, int *fd, int flags) {
    int createFlag = (flags & O_CREAT) ? 1 : 0;
    int writeFlag = (flags & O_RDWR) ? 1 : 0;
    
    *fd = shm_open(name, flags, 0666);
    if (*fd == -1) {
        ERROR_EXIT("shm_open");
    }
    
    if (createFlag) {
        if (ftruncate(*fd, size) == -1) {
            shm_unlink(name);
            close(*fd);
            ERROR_EXIT("ftruncate");
        }
    }
    
    void *addr = mmap(NULL, size, writeFlag ? (PROT_READ | PROT_WRITE) : PROT_READ,
                      MAP_SHARED, *fd, 0);
    if (addr == MAP_FAILED) {
        if (createFlag) {
            shm_unlink(name);
        }
        close(*fd);
        ERROR_EXIT("mmap");
    }
    return addr;
}

SharedMemoryAdt shmAdtOpen(const char *name, size_t size, int flags) {
    SharedMemoryAdt shm;
    shm.size = size;
    shm.addr = mapSharedMemory(name, size, &shm.fd, flags);
    shm.name = strdup(name);
    if (!shm.name) {
        ERROR_EXIT("strdup");
    }
    return shm;
}

void shmAdtClose(SharedMemoryAdt *shm) {
    if (!shm) {
        return;
    }
    
    if (shm->addr) {
        if (munmap(shm->addr, shm->size) == -1) {
            perror("munmap");
        }
        shm->addr = NULL;
    }
    
    if (shm->fd != -1) {
        close(shm->fd);
        shm->fd = -1;
    }
    
    if (shm->name) {
        free(shm->name);
        shm->name = NULL;
    }
}

void shmAdtDestroy(SharedMemoryAdt *shm) {
    if (shm && shm->name) {
        if (shm_unlink(shm->name) == -1) {
            perror("shm_unlink");
        }
    }
    shmAdtClose(shm);
}
