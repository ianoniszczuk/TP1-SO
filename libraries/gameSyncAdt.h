#ifndef GAME_SYNC_ADT_H
#define GAME_SYNC_ADT_H

#include "sharedHeaders.h"
#include "sharedMemoryAdt.h"

typedef struct {
    GameSync *sync;
    SharedMemoryAdt shm;  // Store the SharedMemoryAdt to properly free resources
} GameSyncAdt;

GameSyncAdt initGameSync();
void cleanupGameSync(GameSyncAdt *gameSyncAdt);

#endif
