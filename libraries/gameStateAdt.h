#ifndef GAME_STATE_ADT_H
#define GAME_STATE_ADT_H

#include "sharedHeaders.h"
#include "gameConfig.h"
#include "sharedMemoryAdt.h"

typedef struct {
    GameState *state;
    size_t size;
    SharedMemoryAdt shm;  // Store the SharedMemoryAdt to properly free resources
} GameStateAdt;

GameStateAdt initGameState(size_t board_size, Options *options);
void cleanupGameState(GameStateAdt *gameStateAdt);

#endif