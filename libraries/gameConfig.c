#include "gameConfig.h"
#include <stdio.h>

void printOptions(Options *options) {
    printf("width: %d\n", options->width);
    printf("height: %d\n", options->height);
    printf("delay: %d\n", options->delayMs);
    printf("timeout: %d\n", options->timeoutSec);
    printf("seed: %u\n", options->seed);
    printf("view: %s\n", options->viewPath);
    printf("numPlayers: %d\n", options->numPlayers);
    for (int i = 0; i < options->numPlayers; i++) {
        printf("\t%s\n", options->playerPaths[i]);
    }
} 