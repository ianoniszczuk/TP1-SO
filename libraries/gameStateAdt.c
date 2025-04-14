#include "gameStateAdt.h"
#include "sharedMemoryAdt.h"
#include "errorHandling.h"
#include <fcntl.h> 
#include <sys/mman.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <stdio.h>  

#define GAME_STATE "/game_state"

GameStateAdt initGameState(size_t boardSize, Options *options) {
    GameStateAdt gameStateAdt;
    size_t totalSize = sizeof(GameState) + boardSize;
    
    gameStateAdt.shm = shmAdtOpen(GAME_STATE, totalSize, O_RDWR | O_CREAT);
    gameStateAdt.state = (GameState *)gameStateAdt.shm.addr;
    gameStateAdt.size = totalSize;
    
    gameStateAdt.state->width = options->width;
    gameStateAdt.state->height = options->height;
    gameStateAdt.state->playerCount = options->numPlayers;
    gameStateAdt.state->gameOver = false;

    srand(options->seed);
    for (int i = 0; i < options->width * options->height; i++) {
        gameStateAdt.state->board[i] = (rand() % 9) + 1;
    }
    
    if (fchmod(gameStateAdt.shm.fd, 0444) == -1) {
        ERROR_EXIT("Error setting permissions on game state shared memory");
    }

    return gameStateAdt;
}

void cleanupGameState(GameStateAdt *gameStateAdt) {
    shmAdtDestroy(&gameStateAdt->shm);
}