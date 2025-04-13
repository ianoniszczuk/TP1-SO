#ifndef VIEW_VIEJA_H
#define VIEW_VIEJA_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>
#include "../libraries/sharedHeaders.h"
#include "../libraries/sharedMemoryAdt.h"

/**
 * Structure that groups the shared memory segments used by the view.
 */
typedef struct {
    SharedMemoryAdt gameAdt;   // Mapping for game_state (read-only)
    SharedMemoryAdt syncAdt;   // Mapping for game_sync (read-write)
} ViewMemory;

/**
 * initViewMemory - Initializes the shared memory segments for the view.
 *
 * It calculates the total size for game_state (header + board) and maps each segment
 * using the ADT. The pointers to GameState and GameSync are returned via output parameters.
 *
 * @param width  Board width.
 * @param height Board height.
 * @param game   Output: pointer to the GameState structure.
 * @param sync   Output: pointer to the GameSync structure.
 * @return       Pointer to a ViewMemory structure grouping the used ADT objects.
 */
ViewMemory *initViewMemory(int width, int height, GameState **game, GameSync **sync);

/**
 * cleanupViewMemory - Frees the resources associated with the shared memory segments.
 *
 * @param vm Pointer to the ViewMemory structure.
 */
void cleanupViewMemory(ViewMemory *vm);

/**
 * printBoard - Displays the current state of the game board.
 *
 * @param game Pointer to the GameState structure.
 */
void printBoard(GameState *game);

#endif /* VIEW_VIEJA_H */ 