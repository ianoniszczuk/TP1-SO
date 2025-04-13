#include "viewVieja.h"
#include "errorHandling.h"

// Names for the shared memory segments
#define GAME_STATE "/game_state"
#define GAME_SYNC  "/game_sync"

//Definitions
bool flag = false;

/**
 * initViewMemory - Initializes the shared memory segments for the view.
 *
 * @param width  Board width.
 * @param height Board height.
 * @param game   Output: pointer to the GameState structure.
 * @param sync   Output: pointer to the GameSync structure.
 * @return       Pointer to a ViewMemory structure grouping the used ADT objects.
 */
ViewMemory *initViewMemory(int width, int height, GameState **game, GameSync **sync) {
    ViewMemory *vm = malloc(sizeof(ViewMemory));
    if (!vm) {
        ERROR_EXIT("malloc ViewMemory");
    }

    size_t gameTotalSize = sizeof(GameState) + width * height * sizeof(int);
    vm->gameAdt = shmAdtOpen(GAME_STATE, gameTotalSize, O_RDONLY);
    *game = (GameState *) vm->gameAdt.addr;
    
    if (vm->gameAdt.addr == MAP_FAILED || !*game) {
        free(vm);
        ERROR_EXIT("Failed to open game state");
    }

    vm->syncAdt = shmAdtOpen(GAME_SYNC, sizeof(GameSync), O_RDWR);
    *sync = (GameSync *) vm->syncAdt.addr;
    
    if (vm->syncAdt.addr == MAP_FAILED || !*sync) {
        shmAdtClose(&vm->gameAdt);
        free(vm);
        ERROR_EXIT("Failed to open game sync");
    }

    return vm;
}

/**
 * cleanupViewMemory - Frees the resources associated with the shared memory segments.
 *
 * @param vm Pointer to the ViewMemory structure.
 */
void cleanupViewMemory(ViewMemory *vm) {
    if (vm) {
        shmAdtClose(&vm->gameAdt);  // Use shmAdtClose (not destroy) since we're not the creator
        shmAdtClose(&vm->syncAdt);  // Use shmAdtClose (not destroy) since we're not the creator
        free(vm);
    }
}

/**
 * printBoard - Displays the current state of the game board.
 *
 * @param game Pointer to the GameState structure.
 */
void printBoard(GameState *game) {
    const unsigned short width = game->width;
    const unsigned short height = game->height;
    const unsigned int playerCount = game->playerCount;

    printf("Tablero (%hu x %hu):\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int toPrint = game->board[y*width + x];
            printf("%s%d ", (toPrint < 0 ? "" : " "), toPrint);
        }
        printf("\n");
    }
    printf("\n");

    for(int i = 0; i < playerCount; i++) {
        printf("Jugador  %d, posicion actual : (%d;%d), puntaje : %d, movs invalidos : %d, bloqueado? %d\n", 
               i, 
               game->players[i].x, 
               game->players[i].y, 
               game->players[i].points, 
               game->players[i].invalidMovements, 
               game->players[i].blocked);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int width = atoi(argv[0]);
    int height = atoi(argv[1]);
    GameState *game = NULL;
    GameSync *sync = NULL;

    // Initialize shared memory using our ADT encapsulation
    ViewMemory *vm = initViewMemory(width, height, &game, &sync);
    
    // Main loop: wait for master to signal changes and print the board
    while (!game->gameOver) {
        sem_wait(&sync->printNeeded);  // Wait for master to signal changes
        printBoard(game);
        sem_post(&sync->printDone);    // Signal master that printing is done
    }
    
    // Print final board state
    printBoard(game);
    
    // Clean up resources
    cleanupViewMemory(vm);
    return EXIT_SUCCESS;
}