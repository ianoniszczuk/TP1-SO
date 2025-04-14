#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include "../libraries/sharedHeaders.h"
#include "../libraries/sharedMemoryAdt.h"
#include "../libraries/errorHandling.h"

typedef struct {
    SharedMemoryAdt gameAdt;  
    SharedMemoryAdt syncAdt;  
} PlayerMemory;

PlayerMemory *initPlayerMemory(int width, int height, GameState **game, GameSync **sync);
void cleanupPlayerMemory(PlayerMemory *pm);
int findPlayerNumber(GameState *game);
bool handlePlayerTurn(GameState *game, GameSync *sync, int playerNumber);

#endif