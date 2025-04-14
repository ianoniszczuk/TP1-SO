#include "view.h"
#include "sharedMemoryAdt.h"
#include "sharedHeaders.h"
#include "errorHandling.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

// Names for the shared memory segments
#define GAME_STATE "/game_state"
#define GAME_SYNC  "/game_sync"

//Definiciones
int puntajes[9];
bool flag = false;

const char *colorMap[] = {
    ANSI_COLOR_WHITE,
    ANSI_BRIGHT_BLACK,
    ANSI_COLOR_BLUE,
    ANSI_COLOR_MAGENTA,
    ANSI_COLOR_CYAN,
    ANSI_COLOR_RED,
    ANSI_COLOR_GREEN,
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_BLACK
};

/* Structure that groups the shared memory segments used by the view. */
typedef struct {
    SharedMemoryAdt gameAdt;   // Mapping for game_state (read-only)
    SharedMemoryAdt syncAdt;   // Mapping for game_sync (read-write)
} ViewMemory;

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

    vm->syncAdt = shmAdtOpen(GAME_SYNC, sizeof(GameSync), O_RDWR);
    *sync = (GameSync *) vm->syncAdt.addr;

    return vm;
}

/**
 * cleanupViewMemory - Frees the resources associated with the shared memory segments.
 *
 * @param vm Pointer to the ViewMemory structure.
 */
void cleanupViewMemory(ViewMemory *vm) {
    if (vm) {
        shmAdtClose(&vm->gameAdt);
        shmAdtClose(&vm->syncAdt);
        free(vm);
    }
}

int isHead(GameState * game, int value, int x, int y){
    return (x == game->players[-1*value].x && y == game->players[-1*value].y);
}

void printCellRow(int value, int py, int x, int y, GameState *game){
    for (int px = 0; px < CELL_SIZE; px++){
        if (value <= 0){
            printf("%s", colorMap[-1*value]);
            if (isHead(game, value, x, y)){
                if ((px == 0) && (py == CELL_SIZE/2)){
                    printf("█▓");
                }
                else if ((px == CELL_SIZE-1) && (py == CELL_SIZE/2)){
                    printf("▓█");
                }
                else{
                    printf("%s", game->players[-1*value].blocked ? "░░":"██");
                }
            }
            else {
                printf("██");
            }
        }
        else {
            if ((px == CELL_SIZE/2) && (py == px)){
                printf("%s%s %d", ANSI_BG_BLACK, ANSI_BG_WHITE, value);
            }
            else {
                printf("%s%s  ", ANSI_BG_BLACK, ANSI_BG_WHITE);
            }
        }
        printf(ANSI_COLOR_RESET);
    }
}

void printBoard(GameState *game) {
    const unsigned short width = game->width;
    const unsigned short height = game->height;
    const unsigned int playerCount = game->playerCount;

    printf("%s", ANSI_CLEAR_SCREEN);
    printf("%s", ANSI_CURSOR_HOME);
    
    for (int y = 0; y < height; y++) {
        for (int py = 0; py < CELL_SIZE; py++){
            printf(MARGIN_TAB);
            for (int x = 0; x < width; x++) {
                int toPrint = game->board[y*width + x];
                printCellRow(toPrint, py, x, y, game);
            }
            printf("\n");
        }
    }
    printf("\n");

    for(unsigned int i = 0; i < playerCount; i++) {
        printf(ANSI_BOLD_WHITE);
        printf(MARGIN_TAB);
        printf("Player %s%u%s score: %u", colorMap[i], i, ANSI_BOLD_WHITE, game->players[i].points);
        printf(ANSI_COLOR_RESET);
        printf(" - ");
        printf("%u Invalid movements", game->players[i].invalidMovements);
        printf(ANSI_COLOR_RESET);
        if (game->players[i].blocked){
            printf(ANSI_COLOR_RED);
            printf(" - ");
            printf("Blocked!");
            printf(ANSI_COLOR_RESET);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
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

    // Clean up resources
    cleanupViewMemory(vm);
    return EXIT_SUCCESS;
}