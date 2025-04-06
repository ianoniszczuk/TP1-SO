#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define GAME_STATE "/game_state"
#define GAME_SYNC "/game_sync"

void initializePlayer(GameState **game, GameSync **sync, int *fdState, int *fdSync) {
    // Open shared memory for game state
    *game = openSharedMemory(GAME_STATE, sizeof(GameState), fdState);
    
    // Get dimensions and remap with full size
    unsigned short width = (*game)->width, height = (*game)->height;
    size_t totalSize = sizeof(GameState) + width * height * sizeof(int);
    closeSharedMemory(*game, *fdState, sizeof(GameState));

    // Remap with full size
    *game = openSharedMemory(GAME_STATE, totalSize, fdState);
    
    // Open shared memory for game sync
    *sync = openSharedMemory(GAME_SYNC, sizeof(GameSync), fdSync);
}

void cleanupPlayer(GameState *game, GameSync *sync, int fdState, int fdSync) {
    size_t totalSize = sizeof(GameState) + game->width * game->height * sizeof(int);
    closeSharedMemory(game, fdState, totalSize);
    closeSharedMemory(sync, fdSync, sizeof(GameSync));
}

int findPlayerNumber(GameState *game) {
    for(unsigned int i = 0; i < game->player_count; i++) {
        if(getpid() == game->players[i].pid) {
            return i;
        }
    }
    return -1;
}

void handlePlayerTurn(GameState *game, GameSync *sync, int player_number) {
    // Check if player is blocked
    if(game->players[player_number].blocked) {
        return;
    }

    // Reader-writer synchronization
    sem_wait(&sync->turnstile);
    sem_wait(&sync->readerCountMutex);
    sync->readerCount++;
    if(sync->readerCount == 1) {
        sem_wait(&sync->resourceAccess);
    }
    sem_post(&sync->readerCountMutex);
    sem_post(&sync->turnstile);

    // Generate and send movement
    unsigned char movimiento = rand() % 8;
    if (write(STDOUT_FILENO, &movimiento, sizeof(movimiento)) < 0) {
        ERROR_EXIT("Error writing movement");
    }

    // Release reader-writer lock
    sem_wait(&sync->readerCountMutex);
    sync->readerCount--;
    if(sync->readerCount == 0) {
        sem_post(&sync->resourceAccess);
    }
    sem_post(&sync->readerCountMutex);
}

int main(void) {
    GameState *game = NULL;
    GameSync *sync = NULL;
    int fdState, fdSync;
    int playerNumber;

    // Initialize random seed
    srand(time(NULL));

    // Initialize shared memory and find player number
    initializePlayer(&game, &sync, &fdState, &fdSync);
    playerNumber = findPlayerNumber(game);
    if (playerNumber == -1) {
        cleanupPlayer(game, sync, fdState, fdSync);
        ERROR_EXIT("Player not found in game state");
    }

    // Main game loop
    while (!game->game_over) {
        handlePlayerTurn(game, sync, playerNumber);
        sleep(1);  // Delay between turns
    }

    // Cleanup
    cleanupPlayer(game, sync, fdState, fdSync);
    return EXIT_SUCCESS;
}
