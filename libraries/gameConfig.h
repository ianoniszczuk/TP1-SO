#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdio.h>

#define MAX_PLAYERS 9

typedef struct {
    unsigned short width;
    unsigned short height;
    int delayMs;
    int timeoutSec;
    unsigned int seed;
    char *viewPath;
    int numPlayers;
    char *playerPaths[MAX_PLAYERS];
} Options;

/**
 * Prints the game options to stdout.
 * 
 * @param options Pointer to the Options structure to print.
 */
void printOptions(Options *options);

#endif
