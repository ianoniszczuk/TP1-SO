#include "player.h"
#include "sharedMemoryAdt.h"   
#include "sharedHeaders.h"     
#include "errorHandling.h"     

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

#define GAME_STATE "/game_state"
#define GAME_SYNC  "/game_sync"

PlayerMemory *initPlayerMemory(int width, int height, GameState **game, GameSync **sync) {
    PlayerMemory *pm = malloc(sizeof(PlayerMemory));
    if (!pm) {
        ERROR_EXIT("malloc PlayerMemory");
    }

    size_t gameTotalSize = sizeof(GameState) + width * height * sizeof(int);
    pm->gameAdt = shmAdtOpen(GAME_STATE, gameTotalSize, O_RDONLY);
    *game = (GameState *) pm->gameAdt.addr;

    pm->syncAdt = shmAdtOpen(GAME_SYNC, sizeof(GameSync), O_RDWR);
    *sync = (GameSync *) pm->syncAdt.addr;

    return pm;
}

void cleanupPlayerMemory(PlayerMemory *pm) {
    if (pm) {
        shmAdtClose(&pm->gameAdt);  
        shmAdtClose(&pm->syncAdt);  
        free(pm);
    }
}

int findPlayerNumber(GameState *game) {
    for (unsigned int i = 0; i < game->playerCount; i++) {
        if (getpid() == game->players[i].pid) {
            return i;
        }
    }
    return -1;
}

bool handlePlayerTurn(GameState *game, GameSync *sync, int playerNumber) {
    sem_wait(&sync->turnstile);
    sem_wait(&sync->readerCountMutex); 
    sync->readerCount++;
    if (sync->readerCount == 1) {
        sem_wait(&sync->resourceAccess);
    }
    sem_post(&sync->readerCountMutex);
    sem_post(&sync->turnstile);

    if (game->players[playerNumber].blocked) {
        sem_wait(&sync->readerCountMutex);
        sync->readerCount--;
        if (sync->readerCount == 0) {
            sem_post(&sync->resourceAccess);
        }
        sem_post(&sync->readerCountMutex);
        return false;
    }

    unsigned char movimiento = (unsigned char)(rand() % 8);
    
    if (write(STDOUT_FILENO, &movimiento, sizeof(movimiento)) < 0) {
        ERROR_EXIT("Error writing movement");
    }

    sem_wait(&sync->readerCountMutex);
    sync->readerCount--;
    if (sync->readerCount == 0) {
        sem_post(&sync->resourceAccess);
    }
    sem_post(&sync->readerCountMutex);
    
    return true;
}

int main(int argc, char *argv[]) {
    int width = atoi(argv[0]);
    int height = atoi(argv[1]);
    GameState *game = NULL;
    GameSync *sync = NULL;
    int playerNumber;

    srand(time(NULL) ^ getpid());

    PlayerMemory *pm = initPlayerMemory(width, height, &game, &sync);

    playerNumber = findPlayerNumber(game);
    if (playerNumber == -1) {
        cleanupPlayerMemory(pm);
        ERROR_EXIT("Player not found in game state");
    }

    while (!game->gameOver) {
        if (!handlePlayerTurn(game, sync, playerNumber)) {
            break;
        }
        sleep(1);
    }

    
    cleanupPlayerMemory(pm);
    return EXIT_SUCCESS;
}
