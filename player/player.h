#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include "../libraries/sharedHeaders.h"
#include "../libraries/sharedMemory.h"
#include "../libraries/errorHandling.h"

void initializePlayer(GameState **game, GameSync **sync, int *fdState, int *fdSync, int width2, int height2);
void cleanupPlayer(GameState *game, GameSync *sync, int fd_state, int fd_sync);
int findPlayerNumber(GameState *game);
void handlePlayerTurn(GameState *game, GameSync *sync, int player_number);

#endif