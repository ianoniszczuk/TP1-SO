#ifndef GAME_STATE_ADT_H
#define GAME_STATE_ADT_H

#include "sharedHeaders.h"
#include "gameConfig.h"
#include "sharedMemoryAdt.h"


typedef struct {
    GameState *state;        
    size_t size;            
    SharedMemoryAdt shm;   
} GameStateAdt;

/**
 * @param board_size Size of the game board in bytes
 * @param options Pointer to game options
 * @return GameStateAdt structure
 */
GameStateAdt initGameState(size_t boardSize, Options *options);

/**
 * @param gameStateAdt Pointer to GameStateAdt structure
 */
void cleanupGameState(GameStateAdt *gameStateAdt);

#endif