#ifndef VIEW_H
#define VIEW_H

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

#define CELL_SIZE 3 

#define MARGIN_TAB "\t\t\t\t\t"

#define ANSI_COLOR_BLACK   "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"

#define ANSI_BRIGHT_BLACK   "\x1b[90m"
#define ANSI_BRIGHT_RED     "\x1b[91m"
#define ANSI_BRIGHT_GREEN   "\x1b[92m"
#define ANSI_BRIGHT_YELLOW  "\x1b[93m"
#define ANSI_BRIGHT_BLUE    "\x1b[94m"
#define ANSI_BRIGHT_MAGENTA "\x1b[95m"
#define ANSI_BRIGHT_CYAN    "\x1b[96m"
#define ANSI_BRIGHT_WHITE   "\x1b[97m"

#define ANSI_BOLD_WHITE "\x1b[1;37m"
#define ANSI_BG_BLACK   "\x1b[40m"
#define ANSI_BG_WHITE   "\x1b[37m"

#define ANSI_CLEAR_SCREEN  "\x1b[2J"
#define ANSI_CURSOR_HOME   "\x1b[H"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define GAME_STATE "/game_state"
#define GAME_SYNC  "/game_sync"

typedef struct {
    SharedMemoryAdt gameAdt;   
    SharedMemoryAdt syncAdt;   
} ViewMemory;

/**
 * @param width  Board width.
 * @param height Board height.
 * @param game   Output: pointer to the GameState structure.
 * @param sync   Output: pointer to the GameSync structure.
 * @return       Pointer to a ViewMemory structure grouping the used ADT objects.
 */
ViewMemory *initViewMemory(int width, int height, GameState **game, GameSync **sync);

/**
 * @param vm Pointer to the ViewMemory structure.
 */
void cleanupViewMemory(ViewMemory *vm);

/**
 * @param game  Pointer to the GameState structure.
 * @param value Cell value.
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @return      True if the cell contains a player's head, false otherwise.
 */
int isHead(GameState *game, int value, int x, int y);

/**
 * @param value Value of the cell.
 * @param py    Y position within the cell.
 * @param x     X coordinate of the cell.
 * @param y     Y coordinate of the cell.
 * @param game  Pointer to the GameState structure.
 */
void printCellRow(int value, int py, int x, int y, GameState *game);

/**
 * @param game Pointer to the GameState structure.
 */
void printBoard(GameState *game);

#endif
