#ifndef GAME_STATE_ADT_H
#define GAME_STATE_ADT_H

#include "sharedHeaders.h"
#include "gameConfig.h"

typedef struct {
    GameState *state;
    size_t size;
} GameStateAdt;

GameStateAdt initGameState(size_t board_size, Options *options);
void cleanupGameState(GameStateAdt *gameStateAdt);

#endif