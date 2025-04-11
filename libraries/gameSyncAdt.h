#ifndef GAME_SYNC_ADT_H
#define GAME_SYNC_ADT_H

#include "sharedHeaders.h"

typedef struct {
    GameSync *sync;
} GameSyncAdt;

GameSyncAdt initGameSync();
void cleanupGameSync(GameSyncAdt *gameSyncAdt);

#endif
