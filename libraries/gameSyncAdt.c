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
    SharedMemoryAdt shmAdt = shmAdtOpen(GAME_SYNC, sizeof(GameSync), O_RDWR | O_CREAT);
    gameSyncAdt.sync = (GameSync *)shmAdt.addr;

    sem_init(&(gameSyncAdt.sync->printNeeded), 1, 0);
    sem_init(&(gameSyncAdt.sync->printDone), 1, 0);
    sem_init(&(gameSyncAdt.sync->turnstile), 1, 1);
    sem_init(&(gameSyncAdt.sync->resourceAccess), 1, 1);
    sem_init(&(gameSyncAdt.sync->readerCountMutex), 1, 1);
    gameSyncAdt.sync->readerCount = 0;

    return gameSyncAdt;
}

void cleanupGameSync(GameSyncAdt *gameSyncAdt) {
    sem_destroy(&(gameSyncAdt->sync->printNeeded));
    sem_destroy(&(gameSyncAdt->sync->printDone));
    sem_destroy(&(gameSyncAdt->sync->turnstile));
    sem_destroy(&(gameSyncAdt->sync->resourceAccess));
    sem_destroy(&(gameSyncAdt->sync->readerCountMutex));
    munmap(gameSyncAdt->sync, sizeof(GameSync));
    shm_unlink(GAME_SYNC);
}
