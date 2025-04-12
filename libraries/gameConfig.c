#include "gameConfig.h"
#include <stdio.h>

/**
 * Prints the game options to stdout.
 * 
 * @param options Pointer to the Options structure to print.
 */
void printOptions(Options *options) {
    printf("width: %d\n", options->width);
    printf("height: %d\n", options->height);
    printf("delay: %d\n", options->delay_ms);
    printf("timeout: %d\n", options->timeout_sec);
    printf("seed: %d\n", options->seed);
    printf("view: %s\n", options->view_path);
    printf("numPlayers: %d\n", options->num_players);
    for (int i = 0; i < options->num_players; i++) {
        printf("\t%s\n", options->player_paths[i]);
    }
} 