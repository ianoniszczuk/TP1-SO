#include "gameStateAdt.h"
#include "sharedMemoryAdt.h"
#include "errorHandling.h"
#include <fcntl.h>  // For O_RDWR, O_CREAT
#include <sys/mman.h>  // For mmap, munmap
#include <sys/stat.h>  // For mode constants
#include <unistd.h>  // For shm_unlink
#include <stdio.h>  // For perror

#define GAME_STATE "/game_state"

GameStateAdt initGameState(size_t board_size, Options *options) {
    GameStateAdt gameStateAdt;
    size_t total_size = sizeof(GameState) + board_size;
    
    // Open the shared memory with read-write access for initialization
    SharedMemoryAdt shmAdt = shmAdtOpen(GAME_STATE, total_size, O_RDWR | O_CREAT);
    gameStateAdt.state = (GameState *)shmAdt.addr;
    gameStateAdt.size = total_size;
    
    // Initialize the game state
    gameStateAdt.state->width = options->width;
    gameStateAdt.state->height = options->height;
    gameStateAdt.state->player_count = options->num_players;
    gameStateAdt.state->game_over = false;

    srand(options->seed);
    for (int i = 0; i < options->width * options->height; i++) {
        gameStateAdt.state->board[i] = (rand() % 9) + 1;
    }
    
    // Set the shared memory permissions to read-only for other processes
    if (fchmod(shmAdt.fd, 0444) == -1) {
        perror("Error setting permissions on game state shared memory");
        // Continue despite the error, as this is not fatal
    }

    return gameStateAdt;
}

void cleanupGameState(GameStateAdt *gameStateAdt) {
    munmap(gameStateAdt->state, gameStateAdt->size);
    shm_unlink(GAME_STATE);
}