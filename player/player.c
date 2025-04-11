#include "player.h"
#include "sharedMemoryAdt.h"   // Our ADT encapsulating shared memory operations
#include "sharedHeaders.h"     // Definitions of GameState, GameSync (with snake_case fields)
#include "errorHandling.h"     // Uses ERROR_EXIT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

// Names for the shared memory segments.
#define GAME_STATE "/game_state"
#define GAME_SYNC  "/game_sync"

/* Structure that groups the shared memory segments used by the player. */
typedef struct {
    SharedMemoryAdt gameAdt;   // Mapping for game_state (read-only)
    SharedMemoryAdt syncAdt;   // Mapping for game_sync (read-write)
} PlayerMemory;

/**
 * initPlayerMemory - Initializes the shared memory segments for the player.
 *
 * It calculates the total size for game_state (header + board) and maps each segment
 * using the new ADT. The pointers to GameState and GameSync are returned via output parameters.
 *
 * @param width  Board width.
 * @param height Board height.
 * @param game   Output: pointer to the GameState structure.
 * @param sync   Output: pointer to the GameSync structure.
 * @return       Pointer to a PlayerMemory structure grouping the used ADT objects.
 */
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

/**
 * cleanupPlayerMemory - Frees the resources associated with the shared memory segments.
 *
 * @param pm Pointer to the PlayerMemory structure.
 */
void cleanupPlayerMemory(PlayerMemory *pm) {
    if (pm) {
        shmAdtClose(&pm->gameAdt);
        shmAdtClose(&pm->syncAdt);
        free(pm);
    }
}

/**
 * findPlayerNumber - Searches for the player's index in game_state.
 *
 * Compares the current process PID with the PIDs stored in the players array.
 *
 * @param game Pointer to GameState.
 * @return     Player index if found; -1 otherwise.
 */
int findPlayerNumber(GameState *game) {
    for (unsigned int i = 0; i < game->player_count; i++) {
        if (getpid() == game->players[i].pid) {
            return i;
        }
    }
    return -1;
}

/**
 * handlePlayerTurn - Processes the player's turn.
 *
 * If the player is not blocked, performs reader–writer synchronization
 * using semaphores and sends a random movement (value between 0 and 7) to the master via STDOUT.
 *
 * @param game          Pointer to GameState.
 * @param sync          Pointer to GameSync.
 * @param playerNumber Player index.
 * @return              Boolean indicating if player can continue (false if blocked).
 */
bool handlePlayerTurn(GameState *game, GameSync *sync, int playerNumber) {
    if (game->players[playerNumber].blocked) {
        return false; // Player is blocked, signal to exit
    }

    // Reader-writer synchronization
    sem_wait(&sync->turnstile);
    sem_wait(&sync->readerCountMutex); 
    sync->readerCount++;
    if (sync->readerCount == 1) {
        sem_wait(&sync->resourceAccess);
    }
    sem_post(&sync->readerCountMutex);
    sem_post(&sync->turnstile);

    // Generate and send a random movement (0 to 7)
    unsigned char movimiento = (unsigned char)(rand() % 8);
    
    // Ensure we're writing a single byte correctly
    if (write(STDOUT_FILENO, &movimiento, sizeof(movimiento)) < 0) {
        ERROR_EXIT("Error writing movement");
    }

    sem_wait(&sync->readerCountMutex);
    sync->readerCount--;
    if (sync->readerCount == 0) {
        sem_post(&sync->resourceAccess);
    }
    sem_post(&sync->readerCountMutex);
    
    return true; // Player can continue
}

int main(int argc, char *argv[]) {
    int width = atoi(argv[0]);
    int height = atoi(argv[1]);
    GameState *game = NULL;
    GameSync *sync = NULL;
    int playerNumber;

    srand(time(NULL));

    // Initialize shared memory using our ADT encapsulation
    PlayerMemory *pm = initPlayerMemory(width, height, &game, &sync);

    playerNumber = findPlayerNumber(game);
    if (playerNumber == -1) {
        cleanupPlayerMemory(pm);
        ERROR_EXIT("Player not found in game state");
    }

    // Main game loop
    while (!game->game_over) {
        if (!handlePlayerTurn(game, sync, playerNumber)) {
            // Player is blocked, exit the loop
            break;
        }
        sleep(1);  // Delay between turns
    }

    
    cleanupPlayerMemory(pm);
    return EXIT_SUCCESS;
}
