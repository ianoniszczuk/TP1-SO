#ifndef GAME_SYNC_ADT_H
#define GAME_SYNC_ADT_H

#include "sharedHeaders.h"
#include "sharedMemoryAdt.h"

typedef struct {
    GameSync *sync;       
    SharedMemoryAdt shm;   
} GameSyncAdt;

/**
 * @return GameSyncAdt structure
 */
GameSyncAdt initGameSync();

/**
 * @param gameSyncAdt Pointer to GameSyncAdt structure
 */
void cleanupGameSync(GameSyncAdt *gameSyncAdt);

#endif
