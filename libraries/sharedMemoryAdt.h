#ifndef SHAREDMEMORYADT_H
#define SHAREDMEMORYADT_H

#include <stddef.h>

typedef struct {
    char *name;
    size_t size;
    int fd;
    void *addr;
} SharedMemoryAdt;

/**
 * @param name  Name of the segment (e.g., "/game_state").
 * @param size  Segment size.
 * @param flags Opening flags (e.g., O_RDONLY or O_RDWR; include O_CREAT to create).
 * @return      A SharedMemoryAdt object with the mapped segment.
 */
SharedMemoryAdt shmAdtOpen(const char *name, size_t size, int flags);

/**
 * @param shm Pointer to the SharedMemoryAdt object.
 */
void shmAdtClose(SharedMemoryAdt *shm);

/**
 * @param shm Pointer to the SharedMemoryAdt object.
 */
void shmAdtDestroy(SharedMemoryAdt *shm);

#endif
