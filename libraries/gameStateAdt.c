#include "gameStateAdt.h"
#include "sharedMemoryAdt.h"
#include "errorHandling.h"
#include <fcntl.h>  // For O_RDWR, O_CREAT
#include <sys/mman.h>  // For mmap, munmap
#include <sys/stat.h>  // For mode constants
#include <unistd.h>  // For shm_unlink

#define GAME_STATE "/game_state"

GameStateAdt initGameState(size_t board_size, Options *options) {
    GameStateAdt gameStateAdt;
    size_t total_size = sizeof(GameState) + board_size;
    SharedMemoryAdt shmAdt = shmAdtOpen(GAME_STATE, total_size, O_RDWR | O_CREAT);
    gameStateAdt.state = (GameState *)shmAdt.addr;
    gameStateAdt.size = total_size;

    gameStateAdt.state->width = options->width;
    gameStateAdt.state->height = options->height;
    gameStateAdt.state->player_count = options->num_players;
    gameStateAdt.state->game_over = false;

    srand(options->seed);
    for (int i = 0; i < options->width * options->height; i++) {
        gameStateAdt.state->board[i] = (rand() % 9) + 1;
    }

    return gameStateAdt;
}

void cleanupGameState(GameStateAdt *gameStateAdt) {
    munmap(gameStateAdt->state, gameStateAdt->size);
    shm_unlink(GAME_STATE);
}