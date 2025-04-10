#ifndef SHAREDMEMORYADT_H
#define SHAREDMEMORYADT_H

#include <stddef.h>

/*
 * Structure that encapsulates a shared memory segment.
 * - name: the name of the segment (a copy is stored)
 * - size: mapped segment size
 * - fd: file descriptor associated with the segment
 * - addr: mapped address
 */
typedef struct {
    char *name;
    size_t size;
    int fd;
    void *addr;
} SharedMemoryAdt;

/**
 * shmAdtOpen - Maps (and creates if necessary) a shared memory segment.
 * It implements the same logic as the previous sharedMemory library: using shm_open, ftruncate, mmap.
 * In case of any error the macro ERROR_EXIT is invoked.
 *
 * @param name  Name of the segment (e.g., "/game_state").
 * @param size  Segment size.
 * @param flags Opening flags (e.g., O_RDONLY or O_RDWR; include O_CREAT to create).
 * @return      A SharedMemoryAdt object with the mapped segment.
 */
SharedMemoryAdt shmAdtOpen(const char *name, size_t size, int flags);

/**
 * shmAdtClose - Unmaps and closes the shared memory segment.
 *
 * @param shm Pointer to the SharedMemoryAdt object.
 */
void shmAdtClose(SharedMemoryAdt *shm);

/**
 * shmAdtDestroy - Unlinks (shm_unlink) the shared memory segment and closes it.
 *
 * @param shm Pointer to the SharedMemoryAdt object.
 */
void shmAdtDestroy(SharedMemoryAdt *shm);

#endif // SHAREDMEMORYADT_H
