#include "gameSyncAdt.h"
#include "sharedMemoryAdt.h"
#include "errorHandling.h"
#include <fcntl.h>  // For O_RDWR, O_CREAT
#include <sys/mman.h>  // For mmap, munmap
#include <sys/stat.h>  // For mode constants
#include <unistd.h>  // For shm_unlink

#define GAME_SYNC "/game_sync"

GameSyncAdt initGameSync() {
    GameSyncAdt gameSyncAdt;
    gameSyncAdt.shm = shmAdtOpen(GAME_SYNC, sizeof(GameSync), O_RDWR | O_CREAT);
    gameSyncAdt.sync = (GameSync *)gameSyncAdt.shm.addr;

    sem_init(&(gameSyncAdt.sync->printNeeded), 1, 0);
    sem_init(&(gameSyncAdt.sync->printDone), 1, 0);
    sem_init(&(gameSyncAdt.sync->turnstile), 1, 1);
    sem_init(&(gameSyncAdt.sync->resourceAccess), 1, 1);
    sem_init(&(gameSyncAdt.sync->readerCountMutex), 1, 1);
    gameSyncAdt.sync->readerCount = 0;

    return gameSyncAdt;
}

void cleanupGameSync(GameSyncAdt *gameSyncAdt) {
    // Destroy all semaphores first
    sem_destroy(&(gameSyncAdt->sync->printNeeded));
    sem_destroy(&(gameSyncAdt->sync->printDone));
    sem_destroy(&(gameSyncAdt->sync->turnstile));
    sem_destroy(&(gameSyncAdt->sync->resourceAccess));
    sem_destroy(&(gameSyncAdt->sync->readerCountMutex));
    
    // Use shmAdtDestroy which will properly unmap, close, and free the name
    shmAdtDestroy(&gameSyncAdt->shm);
    // No need to do separate munmap and shm_unlink as they're handled by shmAdtDestroy
}
