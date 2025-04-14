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
#include <signal.h>

//Definiciones
int puntajes[9];
bool flag = false;
ViewMemory *gViewMemory = NULL;
volatile sig_atomic_t receivedSignal = 0;

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

void cleanupAndExit(int sig, int exitCode) {
    if (gViewMemory != NULL) {
        cleanupViewMemory(gViewMemory);
    }
    
    if (sig != 0) {
        fprintf(stderr, "View process terminated by signal %d\n", sig);
    }
    
    exit(exitCode);
}

void signalHandler(int sig) {
    receivedSignal = 1;
    cleanupAndExit(sig, EXIT_FAILURE);
}

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
    // Registramos manejadores para señales
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    
    int width = atoi(argv[0]);
    int height = atoi(argv[1]);
    GameState *game = NULL;
    GameSync *sync = NULL;

    gViewMemory = initViewMemory(width, height, &game, &sync);
    
    while (!game->gameOver && !receivedSignal) {
        sem_wait(&sync->printNeeded); 
        printBoard(game);
        sem_post(&sync->printDone);    
    }

    cleanupViewMemory(gViewMemory);
    gViewMemory = NULL;
    
    return EXIT_SUCCESS;
}